#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "equipmentui.h"
#include "gestionstation/stationui.h"
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>

SkySenseWindow::SkySenseWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setMinimumSize(1100, 700);
    setWindowIcon(QIcon(":/assets/logo/skysense_logo.png"));

    auto *tabs = new QTabWidget(ui->centralWidget);
    auto *layout = new QVBoxLayout(ui->centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tabs);

    auto *equipmentPage = new QWidget(tabs);
    setupEquipmentUi(equipmentPage);
    tabs->addTab(equipmentPage, "Equipment");

    auto *stationPage = new QWidget(tabs);
    GestionStation::setupStationUi(stationPage);
    tabs->addTab(stationPage, "Stations");
}

SkySenseWindow::~SkySenseWindow()
{
    delete ui;
}
