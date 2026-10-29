#ifndef SKYSENSEWINDOW_H
#define SKYSENSEWINDOW_H
#include <QMainWindow>
#include <QList>
#include <QPair>
class QVBoxLayout;
class QHBoxLayout;
class QSpacerItem;
QT_BEGIN_NAMESPACE
namespace Ui { class SkySenseWindow; }
QT_END_NAMESPACE
class SkySenseWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit SkySenseWindow(QWidget *parent = nullptr);
    ~SkySenseWindow();
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    QVBoxLayout *leftLayout = nullptr;
    QHBoxLayout *rightLayout = nullptr;
    QWidget *loginPage = nullptr;
    QList<QPair<QSpacerItem *, int>> responsiveGaps;
    void addResponsiveGap(QVBoxLayout *layout, int height);
    void updateResponsiveLayout();
    Ui::SkySenseWindow *ui;
    void setupLoginUi();
    void setupStyles(QWidget *target);
};
#endif
