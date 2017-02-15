#ifndef MOVIERENDERERWINDOW_H
#define MOVIERENDERERWINDOW_H

#include <QMainWindow>
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

private:
    Ui::MovieRendererWindow *ui;
    MovieRenderer *m_movieRenderer;
};

#endif // MOVIERENDERERWINDOW_H
