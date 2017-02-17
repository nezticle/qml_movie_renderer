#include "movierendererwindow.h"
#include "ui_movierendererwindow.h"
#include <QFileDialog>

#include <QImageWriter>

MovieRendererWindow::MovieRendererWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MovieRendererWindow)
    , m_movieRenderer(new MovieRenderer)
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->verticalLayout);
    connect(ui->openQmlFileButton, SIGNAL(clicked()), this, SLOT(getQmlFile()));
    connect(ui->outputDirectoryPushButton, SIGNAL(clicked()), this, SLOT(getOutputDirectory()));
    connect(ui->outputDirectoryLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkEnableRender()));
    connect(ui->qmlFileLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkEnableRender()));
    connect(ui->renderMovieButton, SIGNAL(clicked()), this, SLOT(renderMovie()));
    connect(m_movieRenderer, SIGNAL(finished()), this, SLOT(handleMovieFinished()));

    //Statusbars
    m_progressBar = new QProgressBar;
    m_progressBar->setValue(0);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    ui->statusbar->addWidget(m_progressBar);
    m_progressBar->setVisible(false);
    connect(m_movieRenderer, SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));

    m_fileProgressBar = new QProgressBar;
    m_fileProgressBar->setValue(0);
    m_fileProgressBar->setMinimum(0);
    m_fileProgressBar->setMaximum(100);
    ui->statusbar->addWidget(m_fileProgressBar);
    m_fileProgressBar->setVisible(false);
    connect(m_movieRenderer, SIGNAL(fileProgressChanged(int)), m_fileProgressBar, SLOT(setValue(int)));

    // Populate Output Format
    for (QByteArray format : QImageWriter::supportedImageFormats()) {
        ui->imageFormatComboBox->addItem(QString::fromLocal8Bit(format));
    }
    // Set Defaults
    ui->imageFormatComboBox->setCurrentIndex(ui->imageFormatComboBox->findText("jpg"));
    ui->outputDirectoryLineEdit->setText(QDir::currentPath());
}

MovieRendererWindow::~MovieRendererWindow()
{
    delete ui;
    delete m_movieRenderer;
}

void MovieRendererWindow::getQmlFile()
{
    QString qmlFile = QFileDialog::getOpenFileName(this, "QML file", QString(), QString("QML Files(*.qml)"));
    ui->qmlFileLineEdit->setText(qmlFile);
}

void MovieRendererWindow::getOutputDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Output Directory");
    ui->outputDirectoryLineEdit->setText(directory);
}

void MovieRendererWindow::checkEnableRender()
{
    if (!ui->qmlFileLineEdit->text().isEmpty() &&
        !ui->outputDirectoryLineEdit->text().isEmpty() &&
        !m_movieRenderer->isRunning()) {
        if (QFile::exists(ui->qmlFileLineEdit->text())) {
            ui->renderMovieButton->setEnabled(true);
            return;
        }
    }

    ui->renderMovieButton->setEnabled(false);
}

void MovieRendererWindow::renderMovie()
{
    //Star Render
    m_movieRenderer->renderMovie(ui->qmlFileLineEdit->text(), ui->outputFilenameLineEdit->text(),
                                 ui->outputDirectoryLineEdit->text(), ui->imageFormatComboBox->currentText(),
                                 QSize(ui->widthSpinBox->value(), ui->heightSpinBox->value()), 1.0,
                                 ui->durationSpinBox->value() * 1000, ui->fpsSpinBox->value());

    //Disable button
    checkEnableRender();

    //Show progress bar
    m_progressBar->setVisible(true);
    m_fileProgressBar->setVisible(true);
}

void MovieRendererWindow::handleMovieFinished()
{
    //Enable Button
    checkEnableRender();

    //Hide progress bar
    m_progressBar->setVisible(false);
    m_fileProgressBar->setVisible(false);
}
