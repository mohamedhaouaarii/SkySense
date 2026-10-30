#pragma once
#include <QObject>
#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QAbstractButton>
#include <QVariantAnimation>

namespace GestionStation {

class AppTheme final : public QObject {
    Q_OBJECT
public:
    enum class Role { Text, Surface, Border };
    static AppTheme *instance();
    static bool dark();
    void initialize(bool persist = true);
    void setDark(bool enabled);
    void applyAll();
    static QColor color(QColor source, Role role);
    static QPen pen(const QPainter &p, QPen source);
    static QPen pen(const QPainter &p, QColor source) { return pen(p,QPen(source)); }
    static QPen pen(const QPainter &p, Qt::GlobalColor source) { return pen(p,QPen(QColor(source))); }
    static QPen pen(const QPainter &p, Qt::PenStyle source) { return pen(p,QPen(source)); }
    static QBrush brush(const QPainter &p, QBrush source);
    static QBrush brush(const QPainter &p, QColor source) { return brush(p,QBrush(source)); }
    static QBrush brush(const QPainter &p, Qt::GlobalColor source) { return brush(p,QBrush(QColor(source))); }
    static QBrush brush(const QPainter &p, Qt::BrushStyle source) { return brush(p,QBrush(source)); }
    static QBrush brush(const QPainter &p, const QGradient &source) { return brush(p,QBrush(source)); }
signals:
    void changed(bool dark);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    explicit AppTheme(QObject *parent = nullptr);
    bool enabled=false, busy=false, persistence=true;
    QPalette lightPalette;
    QString themedStyle(const QString &source) const;
    void applyWidget(QWidget *widget);
    void applyIcons(QWidget *widget);
};

class ThemeSwitch final : public QAbstractButton {
    QVariantAnimation animation;
    qreal position=0;
public:
    explicit ThemeSwitch(QWidget *parent=nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
    void nextCheckState() override;
};

} // namespace GestionStation
