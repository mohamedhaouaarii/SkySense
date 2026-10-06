#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "stationui.h"
#include <QIcon>

SkySenseWindow::SkySenseWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setMinimumSize(900, 520);
    resize(1440, 900);
    setWindowIcon(QIcon(":/assets/stations/station_logo.png"));
    setupStationUi(ui->centralWidget);
}

SkySenseWindow::~SkySenseWindow()
{
    delete ui;
}

