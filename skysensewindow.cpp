#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "serviceui.h"
#include <QIcon>

SkySenseWindow::SkySenseWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setWindowTitle("SkySense - Smart Weather Management | Services");
    setMinimumSize(980, 640);
    setWindowIcon(QIcon(":/assets/logo/skysense_logo.png"));
    setupServiceUi(ui->centralWidget);
}

SkySenseWindow::~SkySenseWindow()
{
    delete ui;
}

