#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QImage>

#include "scribblearea.h"
#include "poissonsolver.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void selectSourceImage();
    void selectDestinationImage();
    void saveImage();
    void penColor();
    void penWidth();
    void penClear();
    void blendROI();
    void about();

private:
    Ui::MainWindow *ui;

    ScribbleArea* _mask_img;
    QImage* _source_img;
    QImage* _dest_img;

    PoissonSolver _solver;

    void createConnections();
    void loadImage(QGraphicsView* view, QImage* image);
};

#endif // MAINWINDOW_H
