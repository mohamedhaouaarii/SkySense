/********************************************************************************
** Form generated from reading UI file 'skysensewindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SKYSENSEWINDOW_H
#define UI_SKYSENSEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SkySenseWindow
{
public:
    QWidget *centralWidget;

    void setupUi(QMainWindow *SkySenseWindow)
    {
        if (SkySenseWindow->objectName().isEmpty())
            SkySenseWindow->setObjectName("SkySenseWindow");
        SkySenseWindow->resize(1646, 932);
        centralWidget = new QWidget(SkySenseWindow);
        centralWidget->setObjectName("centralWidget");
        SkySenseWindow->setCentralWidget(centralWidget);

        retranslateUi(SkySenseWindow);

        QMetaObject::connectSlotsByName(SkySenseWindow);
    } // setupUi

    void retranslateUi(QMainWindow *SkySenseWindow)
    {
        SkySenseWindow->setWindowTitle(QCoreApplication::translate("SkySenseWindow", "SkySense - Smart Weather Management | Services", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SkySenseWindow: public Ui_SkySenseWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SKYSENSEWINDOW_H
