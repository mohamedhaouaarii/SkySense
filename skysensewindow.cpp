#include "skysensewindow.h"
#include "ui_skysensewindow.h"
#include "serviceui.h"
#include <QIcon>
#include <QVBoxLayout>

SkySenseWindow::SkySenseWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::SkySenseWindow)
{
    ui->setupUi(this);
    setMinimumSize(1100, 700);
    setWindowIcon(QIcon(":/assets/logo/skysense_logo.png"));
    // Services page (sidebar + content + all its buttons) shown directly at startup.
    auto *host = new QVBoxLayout(ui->centralWidget);
    host->setContentsMargins(0, 0, 0, 0);
    auto *servicesRoot = new QWidget;
    setupServiceUi(servicesRoot);
    host->addWidget(servicesRoot);
}
SkySenseWindow::~SkySenseWindow() { delete ui; }
