#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRgb>
#include <QColorDialog>
#include <QInputDialog>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _mask_img = new ScribbleArea();
    _source_img = new QImage();
    _dest_img = new QImage();

    createConnections();

    ui->grid_mask->addWidget(_mask_img);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete _mask_img;
    delete _source_img;
    delete _dest_img;
}

void MainWindow::createConnections()
{
    connect(ui->action_source_img_select, SIGNAL(triggered()), this, SLOT(selectSourceImage()));
    connect(ui->action_destination_img_select, SIGNAL(triggered()), this, SLOT(selectDestinationImage()));
    connect(ui->action_save, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(ui->action_pen_width, SIGNAL(triggered()), this, SLOT(penWidth()));
    connect(ui->action_pen_color, SIGNAL(triggered()), this, SLOT(penColor()));
    connect(ui->action_pen_clear, SIGNAL(triggered()), this, SLOT(penClear()));
    connect(ui->action_blend, SIGNAL(triggered()), this, SLOT(blendROI()));
    connect(ui->action_about, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::loadImage(QGraphicsView *view, QImage* image)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("სურათის გახსნა"));

    if (!fileName.isEmpty())
    {
		
		if (!image->load(fileName) || image->width() < view->width() || image->height() < view->height())
        {
            QMessageBox::about(this, "შეტყობინება!", "სურათის გახსნა ვერ მოხერხდა.");
            return;
        }

        QGraphicsScene* scene = new QGraphicsScene();

        QGraphicsScene* old = view->scene();
        if (old != 0) {
            delete old;
        }
		
		if (image->width() < view->width() || image->height() < view->height()) {
			image->scaled(view->width(), view->height());
		}

        scene->addPixmap(QPixmap::fromImage(*image));
        view->setScene(scene);
    }
}

void MainWindow::selectSourceImage()
{
    loadImage(ui->graphicsView_source, _source_img);
}

void MainWindow::selectDestinationImage()
{
    loadImage(ui->graphicsView_destination, _dest_img);
}

void MainWindow::saveImage()
{
    QString initialPath = QDir::currentPath() + "/untitled.jpg";

    QString fileName = QFileDialog::getSaveFileName(this, tr("სურათის შენახვა"),
                                                    initialPath,
                                                    tr("JPEG Image (*.jpeg *.jpg)"));
    if(!fileName.isEmpty()) {
        if (!_dest_img->save(fileName)) {
            QMessageBox::information(this, tr("შეტყობინება"), "სურათის შენახვა ვერ მოხერხდა.");
        }
    }
}

void MainWindow::penColor()
{
    QColor newColor = QColorDialog::getColor(_mask_img->penColor(), this, tr("აირჩიეთ ფუნჯის ფერი"), QColorDialog::DontUseNativeDialog);

    if (newColor.isValid() && newColor.value() & 0xffffff) {
        _mask_img->setPenColor(newColor);
    }
}

void MainWindow::penWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInt(this, windowTitle(), tr("აირჩიეთ ფუნჯის ზომა"), _mask_img->penWidth(), 1, 50, 1, &ok);

    if (ok) {
        _mask_img->setPenWidth(newWidth);
    }
}

void MainWindow::penClear()
{
    _mask_img->clearImage();
}

void MainWindow::blendROI()
{
    if(_source_img->isNull() || _dest_img->isNull()) return;

    int source_offset_x = ui->graphicsView_source->horizontalScrollBar()->value();
    int source_offset_y = ui->graphicsView_source->verticalScrollBar()->value();

    int dest_offset_x = ui->graphicsView_destination->horizontalScrollBar()->value();
    int dest_offset_y = ui->graphicsView_destination->verticalScrollBar()->value();

    int img_size = _mask_img->height() * _mask_img->width();

    int* mask           = (int*) malloc(img_size * sizeof(int));

    uchar* source_red   = (uchar*) malloc(img_size * sizeof(uchar));
    uchar* source_green = (uchar*) malloc(img_size * sizeof(uchar));
    uchar* source_blue  = (uchar*) malloc(img_size * sizeof(uchar));

    uchar* dest_red     = (uchar*) malloc(img_size * sizeof(uchar));
    uchar* dest_green   = (uchar*) malloc(img_size * sizeof(uchar));
    uchar* dest_blue    = (uchar*) malloc(img_size * sizeof(uchar));

    for (int i = 0; i < _mask_img->height(); i++) {
        for (int j = 0; j < _mask_img->width(); j++) {
            QRgb mask_px   = _mask_img->image.pixel(j, i);
            QRgb source_px = _source_img->pixel(source_offset_x + j, source_offset_y + i);
            QRgb dest_px   = _dest_img->pixel(dest_offset_x + j, dest_offset_y + i);

            int idx = i * _mask_img->width() + j;

            mask[idx] = (mask_px & 0xffffff) ? 1 : -1;

            source_red[idx]   = (source_px >> 16) & 0xff;
            source_green[idx] = (source_px >>  8) & 0xff;
            source_blue[idx]  = source_px & 0xff;

            dest_red[idx]     = (dest_px >> 16) & 0xff;
            dest_green[idx]   = (dest_px >>  8) & 0xff;
            dest_blue[idx]    = dest_px & 0xff;
        }
    }

    _solver.blend(mask,
                   source_red,
                   source_green,
                   source_blue,
                   dest_red,
                   dest_green,
                   dest_blue,
                   _mask_img->width(),
                   _mask_img->height(),
                   ui->spinBox_iterations->value(),
				   ui->checkBox_mix_gradients->isChecked());

    for (int i = 0; i < _mask_img->height(); i++) {
        for (int j = 0; j < _mask_img->width(); j++) {
            int idx = i * _mask_img->width() + j;
            _dest_img->setPixel(dest_offset_x + j, dest_offset_y + i, qRgb(dest_red[idx], dest_green[idx], dest_blue[idx]));
        }
    }

    QGraphicsScene* scene = ui->graphicsView_destination->scene();
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(*_dest_img));
    ui->graphicsView_destination->update();

    free(mask);
    free(source_red);
    free(source_green);
    free(source_blue);
    free(dest_red);
    free(dest_green);
    free(dest_blue);
}


void MainWindow::about()
{
    QMessageBox::about(this,
                       tr("პროგრამის შესახებ"),
                       tr("<p>მოცემული პროგრამა წარმოადგენს პუასონის განტოლების მეშვეობით ციფრულ გამოსახულებათა "
                          "დამუშავების ერთ-ერთი ამოცანის (უნაკერო კლონირება/Seamless cloning) რეალიზაციას.</p><p> "
                          "ამოცანის მიზანია სურათში რაიმე სხვა სურათის კონკრეტული ნაწილის ჩასმა ისე, რომ შედეგი "
                          "რაც შეიძლება ბუნებრივად გამოიყურებოდეს, ანუ ჩასმისას მიღებული არტეფაქტების რაოდენობა "
                          "მინიმალური იყოს.</p><p> ავტორი: სანდრო ბარნაბიშვილი</p>"));
}
