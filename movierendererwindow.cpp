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
    if (!ui->qmlFileLineEdit->text().isEmpty() && !ui->outputDirectoryLineEdit->text().isEmpty()) {
        if (QFile::exists(ui->qmlFileLineEdit->text())) {
            ui->renderMovieButton->setEnabled(true);
            return;
        }
    }

    ui->renderMovieButton->setEnabled(false);
}

void MovieRendererWindow::renderMovie()
{

}
