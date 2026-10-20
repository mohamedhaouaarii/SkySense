#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QStringList>
#include <QMouseEvent>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QLabel;
class QPlainTextEdit;
class QCheckBox;
class QStackedWidget;
class QPushButton;
class QSequentialAnimationGroup;

// ============ DATA MODEL (Table 4: Intervention entity) ============
struct Equipment  { QString id, name, station, model; int health; int intervalDays; };
struct Technician { QString name; bool active; bool certified; };
struct Intervention {
    QString id, equipId, title, tech, priority, status, desc, report;
    QDateTime plannedStart, plannedEnd, realStart, realEnd;
    int actionsMask = 0;
};
struct Rank { int idx; int score; QString why; };

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr, bool embedded = false);
    void activateEmbeddedPage();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPoint dragOff; bool dragging = false;
    // --- interface construction
    void seedData();
    QWidget *buildSidebar();
    QWidget *buildWindowControlBar();
    QWidget *buildTopBar();
    QWidget *buildMaintenancePage();
    QWidget *buildDetailCard();
    // --- business logic
    const Equipment *equip(const QString &id) const;
    int idxOf(const QString &id) const;
    QString effStatus(const Intervention &it) const;
    qint64 delayHours(const Intervention &it) const;
    QVector<int> filtered() const;              // B1 sorting + B2 search
    QVector<Rank> ranking() const;              // I1
    // --- rendering
    void refresh();
    void showDetail();
    void animateMaintenancePage();
    // --- actions (buttons)
    void onAdd();
    void onEdit();
    void onDelete();
    void onComplete();
    void exportList();                          // B3 (filtered list)
    void exportSheet();                         // B3 (report)
    void showStats();                           // B4
    void showRanking();                         // I1
    void showPreventive();                      // I2
    void toggleWindowMaximize();

    QVector<Equipment> eq;
    QVector<Technician> techs;
    QVector<Intervention> data;
    QStringList proposed;
    QString selId;
    quint64 persistenceGeneration = 0;

    QStackedWidget *stack = nullptr;
    QWidget *maintenancePage = nullptr;
    QWidget *windowControlBar = nullptr;
    QPushButton *maximizeWindowButton = nullptr;
    QPushButton *topEditButton = nullptr;
    QPushButton *topDeleteButton = nullptr;
    QPushButton *completeInterventionButton = nullptr;
    QSequentialAnimationGroup *maintenanceAnimation = nullptr;
    quint64 maintenanceRevealGeneration = 0;
    bool revealingRows = false;
    bool embeddedMode = false;
    QTableWidget *table = nullptr;
    QLineEdit *search = nullptr;
    QComboBox *fStatus = nullptr, *fPeriod = nullptr, *fSort = nullptr;
    QLabel *kVal[4] = {}, *kSub[4] = {}, *count = nullptr;
    QLabel *dId = nullptr, *dSt = nullptr, *dName = nullptr, *dWhen = nullptr, *dInfo = nullptr;
    QCheckBox *acts[4] = {};
    QPlainTextEdit *notes = nullptr;
};

#endif // MAINWINDOW_H
