#ifndef MOVIERENDERERWINDOW_H
#define MOVIERENDERERWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include "movierenderer.h"


namespace Ui {
class MovieRendererWindow;
}

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

private:
    Ui::MovieRendererWindow *ui;
    QProgressBar *m_progressBar;
    QProgressBar *m_fileProgressBar;
    MovieRenderer *m_movieRenderer;
};

#endif // MOVIERENDERERWINDOW_H
