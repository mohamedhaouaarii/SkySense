#ifndef SKYSENSEWINDOW_H
#define SKYSENSEWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SkySenseWindow; }
QT_END_NAMESPACE

class SkySenseWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SkySenseWindow(QWidget *parent = nullptr);
    ~SkySenseWindow();
private:
    Ui::SkySenseWindow *ui;
};

#endif
