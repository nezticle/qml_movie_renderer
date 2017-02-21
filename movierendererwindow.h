#ifndef MOVIERENDERERWINDOW_H
#define MOVIERENDERERWINDOW_H

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MovieRendererWindow;
}

class QLabel;
class MovieRenderer;
class QProgressBar;
class MovieRendererWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MovieRendererWindow(QWidget *parent = 0);
    ~MovieRendererWindow();

private slots:
    void getQmlFile();
    void getOutputDirectory();
    void checkEnableRender();
    void renderMovie();
    void handleMovieFinished();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setProgressBarsVisible(bool isVisible);
    void readSettings();
    void writeSettings();

    Ui::MovieRendererWindow *ui;
    MovieRenderer *m_movieRenderer;
    QSettings m_settings;

    // Statusbar
    QLabel *m_progressLabel;
    QProgressBar *m_progressBar;
    QLabel *m_fileProgressLabel;
    QProgressBar *m_fileProgressBar;

};

#endif // MOVIERENDERERWINDOW_H
