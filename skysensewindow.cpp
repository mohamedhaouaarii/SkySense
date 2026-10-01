#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "equipmentui.h"
#include <QIcon>

SkySenseWindow::SkySenseWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setMinimumSize(1100, 700);
    setWindowIcon(QIcon(":/assets/logo/skysense_logo.png"));
    setupEquipmentUi(ui->centralWidget);
}

SkySenseWindow::~SkySenseWindow()
{
    delete ui;
}
