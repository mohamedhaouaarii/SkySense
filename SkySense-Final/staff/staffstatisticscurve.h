#pragma once

#include <QHelpEvent>
#include <QMap>
#include <QPainter>
#include <QPolygonF>
#include <QScrollArea>
#include <QToolTip>
#include <QWidget>

// A point is the number of saved people in one category.
class StaffStatisticsCurve final : public QWidget
{
public:
    explicit StaffStatisticsCurve(QWidget *parent = nullptr) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setCounts(const QString &heading, const QMap<QString,int> &counts)
    {
        title = heading;
        labels = counts.keys();
        values = counts.values();
        total = 0;
        maximum = 1;
        for (int count : values) {
            total += count;
            maximum = qMax(maximum, count);
        }
        setMinimumWidth(66 + 92 * labels.size());
        QStringList descriptions;
        for (int i = 0; i < labels.size(); ++i)
            descriptions.append(pointDescription(i));
        setAccessibleName(title);
        setAccessibleDescription(descriptions.join(" ; "));
        updateGeometry();
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QPalette colors = palette();
        const QColor textColor = colors.color(QPalette::Text);
        const QColor accent = colors.color(QPalette::Highlight);
        p.fillRect(rect(), colors.brush(QPalette::Base));
        p.setPen(textColor);
        QFont labelFont = font();
        labelFont.setPixelSize(12);
        QFont titleFont = labelFont;
        titleFont.setBold(true);
        titleFont.setPixelSize(14);
        p.setFont(titleFont);
        p.drawText(QRect(14,3,width()-28,24), Qt::AlignVCenter | Qt::AlignLeft, title);
        if (total == 0) {
            p.setFont(labelFont);
            p.drawText(rect(), Qt::AlignCenter, "No staff to display");
            return;
        }
        const QRectF area = plotArea();
        if (area.height() < 10 || area.width() <= 0) return;
        p.setFont(labelFont);
        const int maxTicks = qBound(2, int(area.height()/30), 6);
        const int tick = qMax(1, (maximum+maxTicks-1)/maxTicks);
        const int ceiling = ((maximum+tick-1)/tick)*tick;
        const auto y = [&](int value) {
            return area.bottom() - area.height()*value/ceiling;
        };
        QColor grid = textColor;
        grid.setAlpha(45);
        for (int value=0; value<=ceiling; value+=tick) {
            p.setPen(grid);
            p.drawLine(QPointF(area.left(),y(value)), QPointF(area.right(),y(value)));
            p.setPen(textColor);
            p.drawText(QRectF(1,y(value)-10,area.left()-8,20),
                       Qt::AlignRight | Qt::AlignVCenter, QString::number(value));
        }

        const qreal step = area.width()/values.size();
        QPolygonF points;
        for (int i=0; i<values.size(); ++i)
            points.append({area.left()+(i+0.5)*step, y(values[i])});
        p.setPen(QPen(accent,2.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        if (points.size()>1) p.drawPolyline(points);
        for (int i=0; i<points.size(); ++i) {
            const QPointF pt = points[i];
            p.setBrush(colors.brush(QPalette::Base));
            p.setPen(QPen(accent,2.4));
            p.drawEllipse(pt,4,4);
            p.setPen(textColor);
            p.setFont(titleFont);
            p.drawText(QRectF(pt.x()-35,pt.y()-23,70,17),
                       Qt::AlignCenter,QString::number(values[i]));
            p.setFont(labelFont);
            const QStringList words = labels[i].split(' ',Qt::SkipEmptyParts);
            QString first, second;
            const QFontMetrics fm(labelFont);
            const int labelWidth = qMax(1,int(step)-8);
            for (const QString &word : words) {
                const QString candidate = first.isEmpty() ? word : first+" "+word;
                if (second.isEmpty() && (first.isEmpty() || fm.horizontalAdvance(candidate)<=labelWidth))
                    first = candidate;
                else
                    second += (second.isEmpty()?QString():QString(" "))+word;
            }
            const auto drawLabel = [&](const QString &line,int offset) {
                p.drawText(QRectF(pt.x()-step/2+4,area.bottom()+6+offset,labelWidth,16),
                           Qt::AlignHCenter | Qt::AlignTop,
                           fm.elidedText(line,Qt::ElideRight,labelWidth));
            };
            drawLabel(first,0);
            if (!second.isEmpty()) drawLabel(second,16);
        }
    }

    bool event(QEvent *event) override
    {
        if (event->type()==QEvent::ToolTip && total>0) {
            const auto *help = static_cast<QHelpEvent *>(event);
            const QRectF area = plotArea();
            if (help->pos().x()>=area.left() && help->pos().x()<area.right()
                    && help->pos().y()>=area.top()-20) {
                const int i=int((help->pos().x()-area.left())*labels.size()/area.width());
                QToolTip::showText(help->globalPos(),pointDescription(i).toHtmlEscaped(),this);
                return true;
            }
        }
        return QWidget::event(event);
    }

private:
    QRectF plotArea() const { return QRectF(rect()).adjusted(38,46,-14,-41); }
    QString pointDescription(int i) const
    {
        const double share = total ? 100.0*values[i]/total : 0;
        return QString("%1 — %2 staff, %3%").arg(labels[i]).arg(values[i]).arg(share,0,'f',1);
    }
    QString title;
    QStringList labels;
    QList<int> values;
    int total=0;
    int maximum=1;
};

inline QScrollArea *staffCurveView(StaffStatisticsCurve *curve, QWidget *parent)
{
    auto *view = new QScrollArea(parent);
    view->setWidgetResizable(true);
    view->setFrameShape(QFrame::NoFrame);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setWidget(curve);
    return view;
}
