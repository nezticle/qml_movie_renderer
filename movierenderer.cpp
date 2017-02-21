/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "movierenderer.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOffscreenSurface>
#include <QScreen>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QCoreApplication>
#include <QEvent>

#include <QtConcurrent>

#include "animationdriver.h"

MovieRenderer::MovieRenderer(QObject *parent)
    : QObject(parent)
    , m_context(nullptr)
    , m_offscreenSurface(nullptr)
    , m_renderControl(nullptr)
    , m_quickWindow(nullptr)
    , m_qmlEngine(nullptr)
    , m_qmlComponent(nullptr)
    , m_rootItem(nullptr)
    , m_fbo(nullptr)
    , m_animationDriver(nullptr)
    , m_status(NotRunning)
{
    QSurfaceFormat format;
    // Qt Quick may need a depth and stencil buffer. Always make sure these are available.
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);

    m_context = new QOpenGLContext;
    m_context->setFormat(format);
    m_context->create();

    m_offscreenSurface = new QOffscreenSurface;
    m_offscreenSurface->setFormat(m_context->format());
    m_offscreenSurface->create();

    m_renderControl = new QQuickRenderControl(this);
    m_quickWindow = new QQuickWindow(m_renderControl);

    m_qmlEngine = new QQmlEngine;
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_quickWindow->incubationController());

    m_context->makeCurrent(m_offscreenSurface);
    m_renderControl->initialize(m_context);
}

void MovieRenderer::renderMovie(const QString &qmlFile, const QString &filename, const QString &outputDirectory, const QString &outputFormat, const QSize &size, qreal devicePixelRatio, int durationMs, int fps)
{
    if (m_status != NotRunning)
        return;

    m_size = size;
    m_dpr = devicePixelRatio;
    setProgress(0);
    setFileProgress(0);
    m_duration = durationMs;
    m_fps = fps;
    m_outputName = filename;
    m_outputDirectory = outputDirectory;
    m_outputFormat = outputFormat;

    if (!loadQML(qmlFile, size))
        return;

    start();
}

MovieRenderer::~MovieRenderer()
{
    m_context->makeCurrent(m_offscreenSurface);
    delete m_renderControl;
    delete m_qmlComponent;
    delete m_quickWindow;
    delete m_qmlEngine;
    delete m_fbo;

    m_context->doneCurrent();

    delete m_offscreenSurface;
    delete m_context;
    delete m_animationDriver;
}

int MovieRenderer::progress() const
{
    return m_progress;
}

void MovieRenderer::start()
{
    m_status = Running;
    createFbo();

    if (!m_context->makeCurrent(m_offscreenSurface))
        return;

    // Render each frame of movie
    m_frames = m_duration / 1000 * m_fps;
    m_animationDriver = new AnimationDriver(1000 / m_fps);
    m_animationDriver->install();
    m_currentFrame = 0;
    m_futureCounter = 0;

    // Start the renderer
    renderNext();
}

void MovieRenderer::cleanup()
{
    m_animationDriver->uninstall();
    delete m_animationDriver;
    m_animationDriver = nullptr;

    destroyFbo();
}

void MovieRenderer::createFbo()
{
    m_fbo = new QOpenGLFramebufferObject(m_size * m_dpr, QOpenGLFramebufferObject::CombinedDepthStencil);
    m_quickWindow->setRenderTarget(m_fbo);
}

void MovieRenderer::destroyFbo()
{
    delete m_fbo;
    m_fbo = nullptr;
}

bool MovieRenderer::loadQML(const QString &qmlFile, const QSize &size)
{
    if (m_qmlComponent != nullptr)
        delete m_qmlComponent;
    m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl(qmlFile), QQmlComponent::PreferSynchronous);

    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    QObject *rootObject = m_qmlComponent->create();
    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    m_rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!m_rootItem) {
        qWarning("run: Not a QQuickItem");
        delete rootObject;
        return false;
    }

    // The root item is ready. Associate it with the window.
    m_rootItem->setParentItem(m_quickWindow->contentItem());

    m_rootItem->setWidth(size.width());
    m_rootItem->setHeight(size.height());

    m_quickWindow->setGeometry(0, 0, size.width(), size.height());

    return true;
}

void static saveImage(const QImage &image, const QString &outputFile)
{
    image.save(outputFile);
}

void MovieRenderer::renderNext()
{

    // Polish, synchronize and render the next frame (into our fbo).
    m_renderControl->polishItems();
    m_renderControl->sync();
    m_renderControl->render();

    m_context->functions()->glFlush();

    m_currentFrame++;

    QString outputFile(m_outputDirectory + QDir::separator() + m_outputName + "_" + QString::number(m_currentFrame) + "." + m_outputFormat);

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
    connect(watcher, SIGNAL(finished()), this, SLOT(futureFinished()));
    watcher->setFuture(QtConcurrent::run(saveImage, m_fbo->toImage(), outputFile));
    m_futures.append(watcher);

    //advance animation
    setProgress(m_currentFrame / (float)m_frames * 100);
    m_animationDriver->advance();

    if (m_currentFrame < m_frames) {
        //Schedule the next update
        QEvent *updateRequest = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, updateRequest);
    } else {
        //Finished
        cleanup();
    }

}

void MovieRenderer::setProgress(int progress)
{
    if (m_progress == progress)
        return;
    m_progress = progress;
    emit progressChanged(progress);
}

void MovieRenderer::futureFinished()
{
    m_futureCounter++;
    setFileProgress(m_futureCounter / (float)m_frames * 100);
    if (m_futureCounter == (m_frames - 1)) {
        qDeleteAll(m_futures);
        m_futures.clear();
        m_status = NotRunning;
        emit finished();
    }
}


bool MovieRenderer::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        renderNext();
        return true;
    }

    return QObject::event(event);
}

bool MovieRenderer::isRunning()
{
    return m_status == Running;
}

int MovieRenderer::fileProgress() const
{
    return m_fileProgress;
}

void MovieRenderer::setFileProgress(int fileProgress)
{
    if (m_fileProgress == fileProgress)
        return;

    m_fileProgress = fileProgress;
    emit fileProgressChanged(fileProgress);
}
