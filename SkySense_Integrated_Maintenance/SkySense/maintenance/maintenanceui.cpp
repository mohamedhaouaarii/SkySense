#include "maintenanceui.h"
#include "mainwindow.h"
#include <QVBoxLayout>

void setupMaintenanceContent(QWidget *root)
{
    root->setObjectName("maintenanceContentRoot");
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *page = new MainWindow(root, true);
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(page, 1);
}
