#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "staffui.h"
#include <QIcon>

SkySenseWindow::SkySenseWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setMinimumSize(1100, 700);
    setWindowIcon(QIcon(":/assets/skysense_logo.png"));
    setupStaffUi(ui->centralWidget);
}

SkySenseWindow::~SkySenseWindow()
{
    delete ui;
}
