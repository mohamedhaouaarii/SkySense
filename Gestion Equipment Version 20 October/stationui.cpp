#include "stationui.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSoundEffect>
#include <QSpinBox>
#include <QDateTime>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>
#include <QTimer>
#include <cmath>
#include <QtMath>
#include <QVBoxLayout>
#include <QVector>
#include <QUrl>
#include <algorithm>
#include <functional>
#include <utility>

namespace {
QLabel *label(const QString &text, const QString &role = QString()) {
    auto *widget = new QLabel(text);
    if (!role.isEmpty())
        widget->setProperty("role", role);
    return widget;
}

QFrame *card(const QString &role = QStringLiteral("card")) {
    auto *widget = new QFrame;
    widget->setProperty("role", role);
    return widget;
}

QIcon stationIcon(const QString &name, const QColor &color = QColor("#091b40")) {
    QPixmap pixels(48, 48);
    pixels.fill(Qt::transparent);
    QPainter painter(&pixels);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(2, 2);
    painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    QPainterPath path;
    if (name == "search") {
        painter.drawEllipse(QRectF(3, 3, 13, 13));
        painter.drawLine(15, 15, 22, 22);
    } else if (name == "export") {
        painter.drawLine(12, 2, 12, 15); painter.drawLine(7, 10, 12, 15);
        painter.drawLine(12, 15, 17, 10); painter.drawLine(3, 16, 3, 21);
        painter.drawLine(3, 21, 21, 21); painter.drawLine(21, 21, 21, 16);
    } else if (name == "chart") {
        painter.drawLine(3, 21, 21, 21); painter.drawLine(4, 21, 4, 4);
        painter.drawRoundedRect(QRectF(7, 13, 3, 6), 1, 1);
        painter.drawRoundedRect(QRectF(12, 9, 3, 10), 1, 1);
        painter.drawRoundedRect(QRectF(17, 5, 3, 14), 1, 1);
    } else if (name == "map") {
        painter.drawEllipse(QRectF(5, 2, 14, 14));
        painter.drawEllipse(QRectF(9, 6, 6, 6));
        painter.drawLine(6, 13, 12, 23); painter.drawLine(12, 23, 18, 13);
    } else if (name == "edit") {
        path.moveTo(4, 17); path.lineTo(15, 6); path.lineTo(19, 10);
        path.lineTo(8, 21); path.lineTo(3, 22); path.closeSubpath();
        painter.drawPath(path); painter.drawLine(13, 8, 17, 12);
        painter.drawLine(4, 17, 8, 21); painter.drawLine(3, 22, 8, 21);
    } else if (name == "delete") {
        painter.drawLine(4, 7, 20, 7); painter.drawLine(9, 4, 15, 4);
        painter.drawLine(6, 8, 7, 21); painter.drawLine(18, 8, 17, 21);
        painter.drawLine(7, 21, 17, 21); painter.drawLine(10, 10, 10, 18);
        painter.drawLine(14, 10, 14, 18);
    } else if (name == "plus") {
        painter.drawLine(3, 12, 21, 12); painter.drawLine(12, 3, 12, 21);
    }
    painter.end();
    pixels.setDevicePixelRatio(2);
    return QIcon(pixels);
}

QPushButton *button(const QString &text, bool primary = false) {
    auto *widget = new QPushButton(text);
    widget->setProperty("primary", primary);
    widget->setMinimumHeight(44);
    widget->setCursor(Qt::PointingHandCursor);
    return widget;
}

QPushButton *stationActionButton(const QString &title, const QString &glyph, bool primary = false) {
    auto *widget = button(title, primary);
    widget->setIcon(stationIcon(glyph, primary ? Qt::white : QColor("#091b40")));
    widget->setIconSize(QSize(20, 20));
    return widget;
}

void showMessage(QWidget *parent, QMessageBox::Icon icon, const QString &title,
                 const QString &message) {
    QMessageBox dialog(icon, title, message, QMessageBox::NoButton, parent);
    auto *ok = dialog.addButton(QString::fromUtf8("✓  OK"), QMessageBox::AcceptRole);
    dialog.setDefaultButton(ok);
    dialog.exec();
}

QLineEdit *searchBox(const QString &hint) {
    auto *widget = new QLineEdit;
    widget->setPlaceholderText(hint);
    widget->addAction(stationIcon("search", QColor("#5d77ad")), QLineEdit::LeadingPosition);
    widget->setMinimumHeight(44);
    return widget;
}

QComboBox *filter(const QString &defaultText, const QStringList &options = {}) {
    auto *widget = new QComboBox;
    widget->addItem(defaultText, QString());
    for (const QString &option : options)
        widget->addItem(option, option);
    widget->setMinimumHeight(44);
    return widget;
}

QString currentTime() {
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(QTime::currentTime(), "HH:mm");
}

class ButtonSoundFilter final : public QObject {
public:
    explicit ButtonSoundFilter(QObject *parent = nullptr) : QObject(parent), clickSound(new QSoundEffect(this)) {
        clickSound->setSource(QUrl("qrc:/sounds/button_click.wav"));
        clickSound->setLoopCount(1);
        clickSound->setVolume(0.4f);
        QObject::connect(clickSound, &QSoundEffect::statusChanged, this, [this] {
            if (pendingClick && clickSound->status() == QSoundEffect::Ready) {
                pendingClick = false;
                clickSound->play();
            }
        });
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        auto *pushButton = qobject_cast<QPushButton *>(watched);
        auto *comboBox = qobject_cast<QComboBox *>(watched);
        const bool isEnabledControl = (pushButton && pushButton->isEnabled()) ||
                                      (comboBox && comboBox->isEnabled());
        if (isEnabledControl) {
            if (event->type() == QEvent::Enter) {
                playClick();
            } else if (event->type() == QEvent::MouseButtonRelease) {
                const auto *mouseEvent = static_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton)
                    playClick();
            } else if (event->type() == QEvent::KeyPress) {
                const auto *keyEvent = static_cast<QKeyEvent *>(event);
                if (!keyEvent->isAutoRepeat() &&
                    (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Return ||
                     keyEvent->key() == Qt::Key_Enter)) {
                    playClick();
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    void playClick() {
        clickSound->stop();
        if (clickSound->status() == QSoundEffect::Ready)
            clickSound->play();
        else if (clickSound->status() == QSoundEffect::Loading)
            pendingClick = true;
    }

    QSoundEffect *clickSound;
    bool pendingClick = false;
};

class Sidebar final : public QWidget {
public:
    Sidebar() {
        setObjectName("sidebar");
        setFixedWidth(170);
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(10, 12, 8, 12);
        layout->setSpacing(5);

        auto *brand = new QLabel;
        brand->setObjectName("brand");
        brand->setPixmap(QPixmap(":/assets/logo/skysense_logo.png").scaled(
            150, 62, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        brand->setAlignment(Qt::AlignCenter);
        layout->addWidget(brand);
        layout->addSpacing(10);

        const QStringList names{"Stations", "Equipment", "Maintenance", "Staff", "Services"};
        const QStringList glyphs{"●", "▤", "⚒", "♟", "▦"};
        for (int i = 0; i < names.size(); ++i) {
            auto *navButton = new QPushButton(glyphs[i] + "    " + names[i]);
            navButton->setProperty("nav", true);
            navButton->setProperty("active", i == 0);
            navButton->setMinimumHeight(30);
            navButton->setCursor(Qt::PointingHandCursor);
            layout->addWidget(navButton);
        }

        layout->addStretch();
        layout->addWidget(label(QString::fromUtf8("●  System Status\n●  Monitoring Active\n   Last updated: ") + currentTime(), "sideStatus"));
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.fillRect(rect(), QColor("#061b35"));
        const QPixmap photo(":/assets/backgrounds/login_background.png");
        const QRectF area(0, height() * 0.36, width(), height() * 0.64);
        QSizeF photoSize = photo.size();
        photoSize.scale(area.size(), Qt::KeepAspectRatioByExpanding);
        painter.save();
        painter.setClipRect(area);
        painter.drawPixmap(QRectF((width() - photoSize.width()) * 0.15,
                                  area.top() + (area.height() - photoSize.height()) / 2,
                                  photoSize.width(), photoSize.height()),
                           photo, photo.rect());
        painter.restore();
        QLinearGradient gradient(0, area.top(), 0, height());
        gradient.setColorAt(0, QColor(6, 27, 53, 255));
        gradient.setColorAt(0.35, QColor(6, 27, 53, 80));
        gradient.setColorAt(0.72, QColor(6, 27, 53, 10));
        gradient.setColorAt(1, QColor(6, 27, 53, 235));
        painter.fillRect(area, gradient);
    }
};

class Bar final : public QWidget {
    int value;
    QColor color;
public:
    Bar(int percentage, QColor fill = QColor("#00bd70")) : value(percentage), color(fill) {
        setMinimumWidth(48);
        setFixedHeight(9);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#e0e7f0"));
        painter.drawRoundedRect(rect(), 5, 5);
        painter.setBrush(color);
        painter.drawRoundedRect(QRect(0, 0, width() * value / 100, height()), 5, 5);
    }
};

class StationBarChart final : public QWidget {
    QStringList labels;
    QVector<int> values;
    QVector<QColor> colors;
    int scaleMaximum;
    QString suffix;

public:
    StationBarChart(QStringList categoryLabels, QVector<int> categoryValues, int maximum,
                    QString valueSuffix, QVector<QColor> barColors)
        : labels(std::move(categoryLabels)), values(std::move(categoryValues)),
          colors(std::move(barColors)), scaleMaximum(qMax(1, maximum)),
          suffix(std::move(valueSuffix)) {
        setMinimumHeight(235);
        setMinimumWidth(210);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        const qreal left = 36;
        const qreal right = width() - 12;
        const qreal top = 18;
        const qreal bottom = height() - 62;
        const qreal plotHeight = bottom - top;
        const qreal plotWidth = right - left;
        if (plotHeight <= 0 || plotWidth <= 0)
            return;

        QFont axisFont = painter.font();
        axisFont.setPointSize(8);
        painter.setFont(axisFont);
        const int tickSteps = qMin(4, scaleMaximum);
        for (int tick = 0; tick <= tickSteps; ++tick) {
            const int value = qRound(scaleMaximum * double(tick) / tickSteps);
            const qreal y = bottom - plotHeight * tick / tickSteps;
            painter.setPen(QPen(QColor("#e2eaf4"), 1));
            painter.drawLine(QPointF(left, y), QPointF(right, y));
            painter.setPen(QColor("#63779b"));
            painter.drawText(QRectF(0, y - 8, left - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(value) + suffix);
        }

        if (labels.isEmpty()) {
            painter.setPen(QColor("#63779b"));
            painter.drawText(rect(), Qt::AlignCenter, "No matching stations");
            return;
        }

        const qreal slotWidth = plotWidth / labels.size();
        const qreal barWidth = qMin(42.0, slotWidth * 0.58);
        QFont valueFont = painter.font();
        valueFont.setBold(true);
        painter.setFont(valueFont);
        for (int index = 0; index < labels.size(); ++index) {
            const int value = qBound(0, values.value(index), scaleMaximum);
            const qreal barHeight = plotHeight * value / scaleMaximum;
            const qreal centerX = left + slotWidth * (index + 0.5);
            const QRectF barRect(centerX - barWidth / 2, bottom - barHeight,
                                 barWidth, barHeight);
            const QColor color = colors.value(index, QColor("#087cff"));
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            if (barHeight > 0)
                painter.drawRoundedRect(barRect, 4, 4);

            painter.setPen(QColor("#26365e"));
            const QString valueText = QString::number(values.value(index)) + suffix;
            painter.drawText(QRectF(centerX - slotWidth / 2, qMax(top, bottom - barHeight - 22),
                                    slotWidth, 18), Qt::AlignCenter, valueText);

            QFont labelFont = painter.font();
            labelFont.setBold(false);
            labelFont.setPointSize(8);
            painter.setFont(labelFont);
            painter.setPen(QColor("#52688e"));
            painter.drawText(QRectF(left + slotWidth * index, bottom + 7,
                                    slotWidth, height() - bottom - 8),
                             Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, labels[index]);
            painter.setFont(valueFont);
        }
    }
};

class StationMetricGlyph final : public QWidget {
    QString name;
    QColor color;
    qreal pulseLevel = 1.0;
public:
    StationMetricGlyph(const QString &icon, const QColor &accent) : name(icon), color(accent) {
        setFixedSize(48, 48);
    }
    void setPulseLevel(qreal level) { pulseLevel = level; update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QColor background = color;
        background.setAlpha(name == "wifi" || name == "offline" ? qRound(22 + 65 * pulseLevel) : 22);
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawEllipse(rect());
        painter.save();
        painter.setOpacity(name == "wifi" || name == "offline" ? 0.55 + 0.45 * pulseLevel : 1.0);
        painter.translate(12, 12);
        painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        if (name == "wifi" || name == "offline") {
            painter.drawArc(QRectF(0, 4, 24, 20), 35 * 16, 110 * 16);
            painter.drawArc(QRectF(5, 9, 14, 12), 35 * 16, 110 * 16);
            painter.drawEllipse(QPointF(12, 20), 1, 1);
            if (name == "offline") painter.drawLine(2, 2, 22, 22);
        } else if (name == "equipment") {
            for (int y : {4, 14}) {
                painter.drawRoundedRect(QRectF(3, y, 18, 6), 1, 1);
                painter.drawPoint(6, y + 3);
                painter.drawLine(16, y + 3, 18, y + 3);
            }
        } else {
            painter.drawEllipse(QRectF(3, 3, 18, 18));
            painter.drawArc(QRectF(7, 7, 10, 10), 0, 180 * 16);
            painter.drawLine(12, 12, 17, 7);
        }
        painter.restore();
    }
};

class StationMiniBars final : public QWidget {
    QColor color;
public:
    explicit StationMiniBars(const QColor &accent) : color(accent) { setFixedSize(30, 42); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        color.setAlpha(65);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        for (int i = 0; i < 4; ++i)
            painter.drawRoundedRect(QRectF(i * width() / 4.0, 32 - i * 9,
                                           qMin(4.0, width() / 6.0), 10 + i * 9), 2, 2);
    }
};

QWidget *metricCard(const QString &title, const QString &value, const QString &tail,
                    const QString &trend, const QString &tone, const QString &glyph,
                    const QString &valueObjectName) {
    auto *frame = card("metric");
    frame->setProperty("tone", tone);
    auto *horizontal = new QHBoxLayout(frame);
    horizontal->setContentsMargins(14, 16, 12, 14);
    horizontal->setSpacing(12);
    const QColor accent = tone == "green" ? QColor("#00b863") :
                          tone == "red" ? QColor("#ff3859") :
                          tone == "purple" ? QColor("#7948ff") : QColor("#0785ff");
    horizontal->addWidget(new StationMetricGlyph(glyph, accent), 0, Qt::AlignTop);

    auto *vertical = new QVBoxLayout;
    vertical->setSpacing(4);
    vertical->addWidget(label(title, "metricTitle"));
    auto *numberLine = new QHBoxLayout;
    numberLine->setSpacing(6);
    auto *number = label(value, "number");
    number->setObjectName(valueObjectName);
    numberLine->addWidget(number);
    numberLine->addWidget(label(tail, "muted"), 1, Qt::AlignBottom);
    vertical->addLayout(numberLine);
    auto *trendLabel = label(trend, "trend");
    trendLabel->setProperty("tone", tone);
    vertical->addWidget(trendLabel);
    horizontal->addLayout(vertical, 1);
    horizontal->addWidget(new StationMiniBars(accent), 0, Qt::AlignVCenter);
    frame->setMinimumHeight(118);
    return frame;
}

QWidget *regional() {
    auto *frame = card();
    frame->setFixedHeight(150);
    auto *vertical = new QVBoxLayout(frame);
    vertical->setContentsMargins(9, 7, 9, 7);
    vertical->setSpacing(5);
    auto *heading = new QHBoxLayout;
    heading->addWidget(label(QString::fromUtf8("◉  Network Availability by Region"), "blockTitle"), 1);
    auto *periodLabel = label("Last 7 Days", "regionCount");
    periodLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    heading->addWidget(periodLabel);
    vertical->addLayout(heading);

    auto *row = new QHBoxLayout;
    row->setSpacing(7);
    auto *summary=label("Availability is estimated from the current demonstration records.","regionCount");
    summary->setWordWrap(true);
    row->addWidget(summary,1);
    vertical->addLayout(row);
    return frame;
}

struct Station {
    int localId = 0;
    QString id;
    QString name;
    QString type;
    QString cityRegion;
    int equipment = 0;
    int availability = 0;
    QString status;
    QString lastUpdate;
};

class StationPage final : public QWidget {
public:
    StationPage() {
        stations = {
            {1,"ALP-03","Alpine Ridge","Mountain Station","Aspen, CO / Rocky Mountains",8,100,"Online","2026-10-18"},
            {2,"COA-01","Coastal Bay","Coastal Station","Santa Cruz, CA / Pacific Coast",6,100,"Online","2026-10-18"},
            {3,"FOR-12","Pine Forest","Forest Station","Bend, OR / Cascade Range",7,86,"Online","2026-10-17"},
            {4,"VAL-07","Green Valley","Valley Station","Boise, ID / Treasure Valley",9,22,"Offline","2026-10-12"},
            {5,"DES-05","Desert View","Desert Station","Phoenix, AZ / Sonoran Desert",5,60,"Maintenance","2026-10-16"},
            {6,"PLA-08","Great Plains","Plains Station","Dodge City, KS / Great Plains",6,96,"Online","2026-10-18"}
        };

        auto *outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(0);

        auto *topBar = card("topbar");
        auto *topLayout = new QHBoxLayout(topBar);
        topLayout->setContentsMargins(28, 12, 24, 12);
        globalSearch = searchBox("Search for a station, city, or region…");
        globalSearch->setObjectName("stationGlobalSearch");
        globalSearch->setMaximumWidth(780);
        topLayout->addWidget(globalSearch, 1);
        topLayout->addStretch();
        QString date = QLocale(QLocale::English, QLocale::UnitedStates).toString(QDate::currentDate(), "ddd, MMM d, yyyy");
        if (!date.isEmpty())
            date[0] = date[0].toUpper();
        auto *calendarIcon = label(QString::fromUtf8("▦"), "dateIcon");
        calendarIcon->setFixedWidth(28);
        topLayout->addWidget(calendarIcon);
        auto *dateBadge = card("dateBadge");
        dateBadge->setFixedWidth(235);
        auto *dateLayout = new QHBoxLayout(dateBadge);
        dateLayout->setContentsMargins(14, 0, 14, 0);
        dateLayout->addWidget(label(date, "topText"));
        topLayout->addWidget(dateBadge);
        topLayout->addSpacing(12);
        QPixmap avatarImage(46, 46);
        avatarImage.fill(Qt::transparent);
        {
            QPainter painter(&avatarImage);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#edf2fb"));
            painter.drawEllipse(0, 0, 45, 45);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor("#6383ba"), 2));
            painter.drawEllipse(18, 10, 10, 11);
            painter.drawArc(QRect(11, 24, 25, 20), 20 * 16, 140 * 16);
        }
        auto *avatar = new QLabel;
        avatar->setPixmap(avatarImage);
        avatar->setFixedSize(46, 46);
        topLayout->addWidget(avatar);
        topLayout->addWidget(label("Alex Martin\nOperations Manager", "user"));
        topBar->hide();
        outer->addWidget(topBar);

        scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        outer->addWidget(scrollArea, 1);

        auto *content = new QWidget;
        auto *body = new QVBoxLayout(content);
        body->setContentsMargins(24, 14, 24, 22);
        body->setSpacing(12);

        auto *heading = new QHBoxLayout;
        auto *copy = new QVBoxLayout;
        copy->setSpacing(0);
        copy->addWidget(label("Stations", "pageTitle"));
        auto *subtitle = label("Track station availability, location, and network performance.", "subtitle");
        subtitle->setWordWrap(true);
        copy->addWidget(subtitle);
        heading->addLayout(copy, 1);

        auto *actions = new QHBoxLayout;
        actions->addStretch();
        auto *addButton = stationActionButton("Add Station", "plus", true);
        auto *editButton = stationActionButton("Edit", "edit");
        auto *deleteButton = stationActionButton("Delete", "delete");
        auto *statisticsButton = stationActionButton("Statistics", "chart");
        auto *exportButton = stationActionButton("Export PDF", "export");
        actions->addWidget(exportButton);
        actions->addWidget(statisticsButton);
        actions->addWidget(editButton);
        actions->addWidget(deleteButton);
        actions->addWidget(addButton);
        heading->addLayout(actions);
        body->addLayout(heading);

        auto *metrics = new QHBoxLayout;
        metrics->setSpacing(10);
        metrics->addWidget(metricCard("Total Stations", "0", "in total", "In this list", "blue", "equipment", "totalValue"), 1);
        auto *onlineMetric=metricCard("Online Stations", "0", "active", "Current status", "green", "wifi", "onlineValue");
        auto *offlineMetric=metricCard("Offline Stations", "0", "to check", "Needs attention", "red", "offline", "offlineValue");
        metrics->addWidget(onlineMetric, 1);
        metrics->addWidget(offlineMetric, 1);
        for (QWidget *metric : {onlineMetric, offlineMetric})
            pulseGlyphs.append(static_cast<StationMetricGlyph *>(metric->layout()->itemAt(0)->widget()));
        metrics->addWidget(metricCard("Average Availability", "0%", "of the network", "Based on listed stations", "purple", "gauge", "availabilityValue"), 1);
        body->addLayout(metrics);

        auto *filters = new QHBoxLayout;
        filters->setSpacing(10);
        tableSearch = searchBox("Search by ID, name, city, or region…");
        regionFilter = filter("All Regions");
        statusFilter = filter("All Statuses", {"Online", "Maintenance", "Offline"});
        typeFilter = filter("All Station Types");
        sortFilter = new QComboBox;
        sortFilter->addItem("Sort by: Name", "name");
        sortFilter->addItem("Sort by: ID", "id");
        sortFilter->addItem("Sort by: Availability", "availability");
        sortFilter->addItem("Sort by: Region", "region");
        sortFilter->addItem("Sort by: Status", "status");
        for (QComboBox *combo : {regionFilter, statusFilter, typeFilter, sortFilter})
            combo->setMinimumHeight(44);
        tableSearch->setMinimumWidth(175);
        regionFilter->setMinimumWidth(95);
        statusFilter->setMinimumWidth(100);
        typeFilter->setMinimumWidth(125);
        sortFilter->setMinimumWidth(140);
        filters->addWidget(tableSearch, 3);
        filters->addWidget(regionFilter, 1);
        filters->addWidget(statusFilter, 1);
        filters->addWidget(typeFilter, 1);
        filters->addWidget(sortFilter, 1);
        body->addLayout(filters);

        table = new QTableWidget(0, 7);
        table->setObjectName("stationsTable");
        table->setHorizontalHeaderLabels({"ID", "Station", "City / Region", "Equipment", "Availability", "Status", "Last Update"});
        table->verticalHeader()->hide();
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->horizontalHeader()->setFixedHeight(40);
        table->verticalHeader()->setDefaultSectionSize(48);
        table->setIconSize(QSize(30, 20));
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setShowGrid(false);
        table->setAlternatingRowColors(false);
        table->setMinimumHeight(40 + 8 * 48 + 4);
        body->addWidget(table);

        auto *situationCard=card();
        situationCard->setObjectName("stationSituationCard");
        situationCard->setMinimumHeight(76);
        auto *situationLayout=new QHBoxLayout(situationCard);
        situationLayout->setContentsMargins(16,10,16,10);
        situationLayout->setSpacing(12);
        stationSituation=label("Station situation: select a row to inspect its available status, location, availability and last update.","subtitle");
        stationSituation->setObjectName("stationSituationSummary");
        stationSituation->setWordWrap(true);
        situationLayout->addWidget(stationSituation,1);
        auto *freshnessSettings=new QVBoxLayout;
        freshnessSettings->setSpacing(2);
        freshnessSettings->addWidget(label("Flag updates older than (days)","tiny"));
        staleDays=new QSpinBox;
        staleDays->setObjectName("stationStaleThresholdDays");
        staleDays->setRange(1,3650);
        staleDays->setValue(30);
        staleDays->setSuffix(" days");
        staleDays->setToolTip("Adjust this value to change the visual alert threshold.");
        freshnessSettings->addWidget(staleDays);
        situationLayout->addLayout(freshnessSettings);
        body->addWidget(situationCard);

        body->addWidget(regional());
        scrollArea->setWidget(content);

        refreshFilterOptions();
        QObject::connect(globalSearch, &QLineEdit::textChanged, this, [this] { refreshTable(); });
        QObject::connect(tableSearch, &QLineEdit::textChanged, this, [this] { refreshTable(); });
        QObject::connect(regionFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(statusFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(typeFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(sortFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(staleDays,qOverload<int>(&QSpinBox::valueChanged),this,[this](int){ refreshTable(); });
        QObject::connect(table,&QTableWidget::itemSelectionChanged,this,[this]{ updateSituation(); });
        QObject::connect(addButton, &QPushButton::clicked, this, [this] { addStation(); });
        QObject::connect(editButton, &QPushButton::clicked, this, [this] { editStation(); });
        QObject::connect(deleteButton, &QPushButton::clicked, this, [this] { deleteStation(); });
        QObject::connect(statisticsButton, &QPushButton::clicked, this, [this] { showStatistics(); });
        QObject::connect(exportButton, &QPushButton::clicked, this, [this] { exportPdf(); });
        refreshTable();
        auto *pulseTimer=new QTimer(this);
        pulseTimer->setInterval(75);
        QObject::connect(pulseTimer,&QTimer::timeout,this,[this,phase=0.0]() mutable {
            if(!isVisible()) return;
            phase+=0.17;
            const qreal level=(1.0+std::sin(phase))*0.5;
            for(const auto &glyph:pulseGlyphs)
                if(glyph) glyph->setPulseLevel(level);
            for(const auto &entry:pulseStatuses) {
                if(!entry.first) continue;
                const bool offline=entry.second=="Offline", maintenance=entry.second=="Maintenance";
                const QColor base(offline?"#fee2e2":maintenance?"#fef3c7":"#dcfce7");
                const QColor accent(offline?"#ef4444":maintenance?"#d97706":"#16a34a");
                const qreal mix=0.02+0.10*level;
                const QColor fill(qRound(base.red()*(1-mix)+accent.red()*mix),
                                  qRound(base.green()*(1-mix)+accent.green()*mix),
                                  qRound(base.blue()*(1-mix)+accent.blue()*mix));
                entry.first->setStyleSheet(QString("background:%1;color:%2;border-radius:14px;font-size:13px;border:2px solid %3;")
                                           .arg(fill.name(),accent.name(),base.name()));
            }
        });
        pulseTimer->start();
    }

    void showStations() {
        scrollArea->verticalScrollBar()->setValue(0);
    }

    void showStatistics() {
        struct RegionSummary {
            int stations = 0;
            int online = 0;
            int availability = 0;
        };

        int online = 0;
        int maintenance = 0;
        int offline = 0;
        int totalEquipment = 0;
        int totalAvailability = 0;
        QMap<QString, RegionSummary> regions;
        for (int stationIndex : visibleStationIndices) {
            const Station &station = stations[stationIndex];
            if (station.status == "Online")
                ++online;
            else if (station.status == "Maintenance")
                ++maintenance;
            else if (station.status == "Offline")
                ++offline;
            totalEquipment += station.equipment;
            totalAvailability += station.availability;

            const QString stationArea = stationRegion(station);
            const QString region = stationArea.isEmpty() ? QStringLiteral("Unspecified") : stationArea;
            RegionSummary &summary = regions[region];
            ++summary.stations;
            summary.online += station.status == "Online";
            summary.availability += station.availability;
        }

        const int total = visibleStationIndices.size();
        const int averageAvailability = total == 0
                                            ? 0
                                            : qRound(double(totalAvailability) / total);

        QDialog dialog(this);
        dialog.setWindowTitle("Nour's Station - Statistics");
        dialog.resize(860, 600);
        dialog.setMinimumSize(700, 500);
        auto *layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(18, 16, 18, 14);
        layout->setSpacing(10);

        layout->addWidget(label("Station Statistics", "pageTitle"));
        layout->addWidget(label(
            QString("Based on the %1 station%2 currently shown after search and filters.")
                .arg(total).arg(total == 1 ? "" : "s"),
            "subtitle"));

        auto *metrics = new QHBoxLayout;
        metrics->setSpacing(8);
        metrics->addWidget(metricCard("Stations", QString::number(total), "shown",
                                      "Current results", "blue", "equipment", "statsTotal"), 1);
        metrics->addWidget(metricCard("Online", QString::number(online), "active",
                                      "Current results", "green", "wifi", "statsOnline"), 1);
        metrics->addWidget(metricCard("Equipment", QString::number(totalEquipment), "items",
                                      "Across shown stations", "purple", "equipment", "statsEquipment"), 1);
        metrics->addWidget(metricCard("Availability", QString::number(averageAvailability) + "%",
                                      "average", "Across shown stations", "red", "gauge", "statsAvailability"), 1);
        layout->addLayout(metrics);

        auto *columns = new QHBoxLayout;
        columns->setSpacing(12);

        auto *statusColumn = new QVBoxLayout;
        statusColumn->setSpacing(8);
        statusColumn->addWidget(label("Status Breakdown", "mapTitle"));
        auto *statusCard = card();
        auto *statusLayout = new QVBoxLayout(statusCard);
        statusLayout->setContentsMargins(8, 8, 8, 8);
        statusLayout->addWidget(new StationBarChart(
            {"Online", "Maintenance", "Offline"}, {online, maintenance, offline},
            qMax(1, total), "", {QColor("#00bd70"), QColor("#ffb000"), QColor("#fa354b")}));
        statusColumn->addWidget(statusCard);
        columns->addLayout(statusColumn, 1);

        auto *regionColumn = new QVBoxLayout;
        regionColumn->setSpacing(8);
        regionColumn->addWidget(label("Regional Availability", "mapTitle"));
        QStringList regionNames;
        QVector<int> regionAvailability;
        QVector<QColor> regionColors;
        for (auto it = regions.constBegin(); it != regions.constEnd(); ++it) {
            const RegionSummary &summary = it.value();
            regionNames.append(it.key());
            regionAvailability.append(qRound(double(summary.availability) / summary.stations));
            regionColors.append(QColor("#087cff"));
        }
        auto *regionCard = card();
        auto *regionChartLayout = new QVBoxLayout(regionCard);
        regionChartLayout->setContentsMargins(8, 8, 8, 8);
        regionChartLayout->addWidget(new StationBarChart(
            regionNames, regionAvailability, 100, "%", regionColors));
        regionColumn->addWidget(regionCard, 1);
        columns->addLayout(regionColumn, 2);
        layout->addLayout(columns, 1);

        auto *actions = new QHBoxLayout;
        actions->addStretch();
        auto *close = button(QString::fromUtf8("×  Close"), true);
        actions->addWidget(close);
        layout->addLayout(actions);
        QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
        dialog.exec();
    }

private:
    QVector<QPointer<StationMetricGlyph>> pulseGlyphs;
    QVector<QPair<QPointer<QLabel>,QString>> pulseStatuses;
    QVector<Station> stations;
    QVector<int> visibleStationIndices;
    QTableWidget *table = nullptr;
    QScrollArea *scrollArea = nullptr;
    QLineEdit *globalSearch = nullptr;
    QLineEdit *tableSearch = nullptr;
    QComboBox *regionFilter = nullptr;
    QComboBox *statusFilter = nullptr;
    QComboBox *typeFilter = nullptr;
    QComboBox *sortFilter = nullptr;
    QLabel *stationSituation = nullptr;
    QSpinBox *staleDays = nullptr;

    QDateTime stationUpdateDate(const QString &value) const {
        const QString v=value.trimmed();
        const QDateTime iso=QDateTime::fromString(v,Qt::ISODate); if(iso.isValid()) return iso;
        for(const QString &format:{QString("yyyy-MM-dd HH:mm:ss"),QString("yyyy-MM-dd"),QString("MMM d, yyyy HH:mm"),QString("MMM d, yyyy")}) {
            const QDateTime dt=QDateTime::fromString(v,format); if(dt.isValid()) return dt;
            const QDate d=QDate::fromString(v,format); if(d.isValid()) return QDateTime(d,QTime(0,0));
        }
        return {};
    }
    bool updateIsStale(const QString &value) const {
        const QDateTime updated=stationUpdateDate(value);
        return updated.isValid() && updated.daysTo(QDateTime::currentDateTime())>staleDays->value();
    }
    void updateSituation() {
        if(!stationSituation || !table || !staleDays) return;
        const int row=table->currentRow();
        if(row<0 || row>=visibleStationIndices.size()) {
            stationSituation->setText("Station situation: select a row to inspect its available status, location, availability and last update.");
            stationSituation->setStyleSheet(QString()); return;
        }
        const Station &st=stations[visibleStationIndices[row]];
        const QDateTime updated=stationUpdateDate(st.lastUpdate);
        const bool stale=updateIsStale(st.lastUpdate);
        const QString freshness=!updated.isValid()?"Last update date unavailable":stale?QString("Stale update — %1 days old").arg(updated.daysTo(QDateTime::currentDateTime())):"Update within selected threshold";
        stationSituation->setText(QString("%1  |  %2  |  %3  |  %4% availability  |  %5 equipment  |  Last update: %6  |  %7")
            .arg(st.name,st.type,st.cityRegion.simplified()).arg(st.availability).arg(st.equipment).arg(st.lastUpdate.isEmpty()?QString("Unknown"):st.lastUpdate,freshness));
        stationSituation->setStyleSheet(stale?"color:#b42318;font-weight:600;":!updated.isValid()?"color:#a15c00;font-weight:600;":"color:#264788;");
    }

    void exportPdf() {
        const QString suggestedName = "stations-" + QDate::currentDate().toString("yyyyMMdd") + ".pdf";
        QString fileName = QFileDialog::getSaveFileName(
            this, "Export station report", suggestedName, "PDF files (*.pdf)");
        if (fileName.isEmpty())
            return;
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
            fileName += ".pdf";

        QPdfWriter writer(fileName);
        writer.setTitle("Weather Station Report");
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageOrientation(QPageLayout::Landscape);
        writer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
        writer.setResolution(96);

        QPainter painter(&writer);
        if (!painter.isActive()) {
            showMessage(this, QMessageBox::Warning, "PDF Export",
                        "The PDF file could not be created.");
            return;
        }

        const QRect page = writer.pageLayout().paintRectPixels(writer.resolution());
        const int left = page.left();
        const int width = page.width();
        int y = page.top();

        int onlineCount = 0;
        int offlineCount = 0;
        int totalAvailability = 0;
        for (int stationIndex : visibleStationIndices) {
            const Station &station = stations[stationIndex];
            onlineCount += station.status == "Online";
            offlineCount += station.status == "Offline";
            totalAvailability += station.availability;
        }
        const int averageAvailability = visibleStationIndices.isEmpty()
                                            ? 0
                                            : qRound(double(totalAvailability) / visibleStationIndices.size());

        const QStringList headers{"Station ID", "Station", "City / Region", "Equipment",
                                  "Availability", "Status", "Last Update"};
        const qreal columnRatios[]{0.10, 0.18, 0.18, 0.10, 0.13, 0.12, 0.19};
        int columnWidths[7];
        int usedWidth = 0;
        for (int column = 0; column < 6; ++column) {
            columnWidths[column] = qRound(width * columnRatios[column]);
            usedWidth += columnWidths[column];
        }
        columnWidths[6] = width - usedWidth;

        const auto drawTableHeader = [&](int top) {
            const int headerHeight = 34;
            painter.fillRect(QRect(left, top, width, headerHeight), QColor("#061b35"));
            QFont headerFont("Segoe UI");
            headerFont.setPointSizeF(8.5);
            headerFont.setBold(true);
            painter.setFont(headerFont);
            painter.setPen(Qt::white);
            int x = left;
            for (int column = 0; column < 7; ++column) {
                const QRect cell(x, top, columnWidths[column], headerHeight);
                painter.drawText(cell.adjusted(6, 2, -6, -2),
                                 Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap,
                                 headers[column]);
                x += columnWidths[column];
            }
            return top + headerHeight;
        };

        QFont titleFont("Segoe UI");
        titleFont.setPointSizeF(20);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.setPen(QColor("#10152e"));
        painter.drawText(QRect(left, y, width, 32), Qt::AlignVCenter | Qt::AlignLeft,
                         "Weather Station Report");
        y += 38;

        QFont detailFont("Segoe UI");
        detailFont.setPointSizeF(9.5);
        painter.setFont(detailFont);
        painter.setPen(QColor("#60749c"));
        painter.drawText(QRect(left, y, width, 20), Qt::AlignVCenter | Qt::AlignLeft,
                         "Generated " + QDate::currentDate().toString("MMMM d, yyyy"));
        y += 27;

        QFont summaryFont("Segoe UI");
        summaryFont.setPointSizeF(10);
        summaryFont.setBold(true);
        painter.setFont(summaryFont);
        painter.setPen(QColor("#26365e"));
        const QString summary = QString("%1 stations shown   •   %2 online   •   %3 offline   •   %4% average availability")
                                    .arg(visibleStationIndices.size())
                                    .arg(onlineCount)
                                    .arg(offlineCount)
                                    .arg(averageAvailability);
        painter.drawText(QRect(left, y, width, 22), Qt::AlignVCenter | Qt::AlignLeft, summary);
        y += 30;
        y = drawTableHeader(y);

        const int rowHeight = 44;
        QFont cellFont("Segoe UI");
        cellFont.setPointSizeF(8.5);
        painter.setFont(cellFont);
        for (int row = 0; row < visibleStationIndices.size(); ++row) {
            if (y + rowHeight > page.bottom() - 16) {
                writer.newPage();
                y = page.top();
                QFont continuationFont("Segoe UI");
                continuationFont.setPointSizeF(13);
                continuationFont.setBold(true);
                painter.setFont(continuationFont);
                painter.setPen(QColor("#10152e"));
                painter.drawText(QRect(left, y, width, 26), Qt::AlignVCenter | Qt::AlignLeft,
                                 "Weather Station Report — continued");
                y += 32;
                painter.setFont(cellFont);
                y = drawTableHeader(y);
            }

            const Station &station = stations[visibleStationIndices[row]];
            const QString values[]{station.id, station.name + '\n' + station.type, station.cityRegion,
                                   QString::number(station.equipment),
                                   QString::number(station.availability) + "%", station.status,
                                   station.lastUpdate};
            const QRect rowRect(left, y, width, rowHeight);
            painter.fillRect(rowRect, row % 2 == 0 ? QColor("#f5f8fd") : QColor(Qt::white));
            painter.setPen(QColor("#e3ebf5"));
            painter.drawRect(rowRect);

            int x = left;
            for (int column = 0; column < 7; ++column) {
                const QRect cell(x, y, columnWidths[column], rowHeight);
                const int alignment = column == 3
                                          ? Qt::AlignCenter | Qt::TextWordWrap
                                          : Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap;
                painter.setPen(column == 5
                                   ? (station.status == "Maintenance" ? QColor("#d58b00")
                                      : station.status == "Offline" ? QColor("#e34754")
                                                                     : QColor("#00a866"))
                                   : QColor("#14234b"));
                painter.drawText(cell.adjusted(6, 2, -6, -2), alignment, values[column]);
                x += columnWidths[column];
            }
            y += rowHeight;
        }

        if (visibleStationIndices.isEmpty()) {
            painter.setPen(QColor("#60749c"));
            painter.drawText(QRect(left, y + 12, width, 30), Qt::AlignVCenter | Qt::AlignLeft,
                             "No stations match the current filters.");
        }

        painter.end();
        showMessage(this, QMessageBox::Information, "PDF Export",
                    "The report was saved to:\n" + fileName);
    }

    static QString stationRegion(const Station &station) {
        return station.cityRegion.section('\n', -1).trimmed();
    }

    static void replaceFilterOptions(QComboBox *combo, const QString &allLabel,
                                     const QStringList &options) {
        const QString previousSelection = combo->currentText();
        const QSignalBlocker blocker(combo);
        combo->clear();
        combo->addItem(allLabel, QString());
        for (const QString &option : options)
            combo->addItem(option, option);
        const int previousIndex = combo->findText(previousSelection);
        combo->setCurrentIndex(previousIndex < 0 ? 0 : previousIndex);
    }

    void refreshFilterOptions() {
        QStringList regions;
        QStringList types;
        for (const Station &station : stations) {
            const QString region = stationRegion(station);
            if (!region.isEmpty() && !regions.contains(region, Qt::CaseInsensitive))
                regions.append(region);
            const QString type = station.type.trimmed();
            if (!type.isEmpty() && !types.contains(type, Qt::CaseInsensitive))
                types.append(type);
        }
        regions.sort(Qt::CaseInsensitive);
        types.sort(Qt::CaseInsensitive);
        replaceFilterOptions(regionFilter, "All Regions", regions);
        replaceFilterOptions(typeFilter, "All Station Types", types);
    }

    int selectedStationRow(const QString &action) {
        const int row = table->currentRow();
        if (row < 0 || row >= visibleStationIndices.size()) {
            showMessage(this, QMessageBox::Information, action,
                        "Select a station from the table first.");
            return -1;
        }
        return visibleStationIndices[row];
    }

    

    

    void openStationForm(bool editing) {
        const int row = editing ? selectedStationRow("Edit Station") : -1;
        if (editing && row < 0)
            return;

        QDialog dialog(this);
        dialog.setWindowTitle(editing ? "Edit Station" : "Add Station");
        dialog.setMinimumWidth(440);
        auto *layout = new QVBoxLayout(&dialog);
        auto *form = new QFormLayout;
        auto *id = new QLineEdit(&dialog);
        auto *name = new QLineEdit(&dialog);
        auto *type = new QLineEdit(&dialog);
        auto *city = new QLineEdit(&dialog);
        auto *region = new QLineEdit(&dialog);
        id->setMaxLength(30);name->setMaxLength(100);type->setMaxLength(50);city->setMaxLength(100);region->setMaxLength(100);
        auto *derived = new QLabel("Equipment count and availability are calculated in preview mode equipment records.",&dialog);
        derived->setWordWrap(true);
        auto *status = new QComboBox(&dialog);
        status->addItems({"Online", "Maintenance", "Offline"});

        if (editing) {
            const Station &station = stations[row];
            id->setText(station.id);
            name->setText(station.name);
            type->setText(station.type);
            const QStringList locationParts = station.cityRegion.split('\n');
            city->setText(locationParts.value(0));
            region->setText(locationParts.value(1));
            status->setCurrentText(station.status);
        }

        form->addRow("Station ID:", id);
        form->addRow("Name:", name);
        form->addRow("Station Type:", type);
        form->addRow("City:", city);
        form->addRow("Region:", region);
        form->addRow("Network metrics:", derived);
        form->addRow("Status:", status);
        layout->addLayout(form);

        auto *actions = new QHBoxLayout;
        actions->addStretch();
        auto *cancel = button(QString::fromUtf8("×  Cancel"));
        auto *save = button(editing ? QString::fromUtf8("✓  Save")
                                    : QString::fromUtf8("＋  Add"), true);
        actions->addWidget(cancel);
        actions->addWidget(save);
        layout->addLayout(actions);

        QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(save, &QPushButton::clicked, &dialog, [&] {
            const QString cleanId = id->text().trimmed();
            const QString cleanName = name->text().trimmed();
            if (cleanId.isEmpty() || cleanName.isEmpty()) {
            showMessage(&dialog, QMessageBox::Warning, "Required Fields",
                        "The station ID and name are required.");
                return;
            }
            for (int index = 0; index < stations.size(); ++index) {
                if (index != row && stations[index].id.compare(cleanId, Qt::CaseInsensitive) == 0) {
                    showMessage(&dialog, QMessageBox::Warning, "ID Already Used",
                                "A station already uses this ID.");
                    return;
                }
            }
            dialog.accept();
        });

        if (dialog.exec() != QDialog::Accepted)
            return;

        Station station;
        if(editing)station.localId=stations[row].localId;
        station.id = id->text().trimmed();
        station.name = name->text().trimmed();
        station.type = type->text().trimmed();
        station.cityRegion = city->text().trimmed();
        if (!region->text().trimmed().isEmpty())
            station.cityRegion += '\n' + region->text().trimmed();
        station.status = status->currentText();
        
            station.localId = editing ? stations[row].localId : stations.size()+1;
            station.lastUpdate=QDate::currentDate().toString(Qt::ISODate);
            station.availability=station.status=="Online"?100:station.status=="Maintenance"?60:0;
            if (editing) { station.equipment=stations[row].equipment; stations[row]=station; }
            else stations.append(station);
        
        int changedStationIndex=-1;for(int i=0;i<stations.size();++i)if(stations[i].id==station.id){changedStationIndex=i;break;}
        refreshFilterOptions();
        refreshTable();
        const int visibleRow = visibleStationIndices.indexOf(changedStationIndex);
        if (visibleRow >= 0)
            table->selectRow(visibleRow);
    }

    void addStation() {
        openStationForm(false);
    }

    void editStation() {
        openStationForm(true);
    }

    void deleteStation() {
        const int row = selectedStationRow("Delete Station");
        if (row < 0)
            return;

        QMessageBox confirmation(this);
        confirmation.setIcon(QMessageBox::Question);
        confirmation.setWindowTitle("Delete Station");
        confirmation.setText(QString("Delete station “%1”?").arg(stations[row].name));
        auto *cancelButton = confirmation.addButton(QString::fromUtf8("×  Cancel"), QMessageBox::RejectRole);
        auto *deleteButton = confirmation.addButton(QString::fromUtf8("×  Delete"), QMessageBox::DestructiveRole);
        confirmation.setDefaultButton(cancelButton);
        confirmation.exec();
        if (confirmation.clickedButton() != deleteButton)
            return;

         else stations.removeAt(row);
        refreshFilterOptions();
        refreshTable();
    }

    void refreshTable() {
        pulseStatuses.clear();
        table->clearSelection();
        QVector<int> matchingStations;
        const QString globalQuery = globalSearch->text().trimmed();
        const QString tableQuery = tableSearch->text().trimmed();
        const QString selectedRegion = regionFilter->currentData().toString();
        const QString selectedStatus = statusFilter->currentData().toString();
        const QString selectedType = typeFilter->currentData().toString();
        for (int index = 0; index < stations.size(); ++index) {
            const Station &station = stations[index];
            const QString searchableText = station.id + " " + station.name + " " + station.type + " " + station.cityRegion;
            if ((!globalQuery.isEmpty() && !searchableText.contains(globalQuery, Qt::CaseInsensitive)) ||
                (!tableQuery.isEmpty() && !searchableText.contains(tableQuery, Qt::CaseInsensitive)))
                continue;
            if (!selectedRegion.isEmpty() && stationRegion(station) != selectedRegion)
                continue;
            if (!selectedStatus.isEmpty() && station.status != selectedStatus)
                continue;
            if (!selectedType.isEmpty() && station.type != selectedType)
                continue;
            matchingStations.append(index);
        }

        const QString sortKey = sortFilter->currentData().toString();
        std::sort(matchingStations.begin(), matchingStations.end(), [this, &sortKey](int left, int right) {
            const Station &a = stations[left];
            const Station &b = stations[right];
            if (sortKey == "id")
                return QString::compare(a.id, b.id, Qt::CaseInsensitive) < 0;
            if (sortKey == "availability")
                return a.availability > b.availability;
            if (sortKey == "region") {
                const int regionOrder = QString::compare(stationRegion(a), stationRegion(b), Qt::CaseInsensitive);
                if (regionOrder != 0)
                    return regionOrder < 0;
                return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
            }
            if (sortKey == "status") {
                const int statusOrder = QString::compare(a.status, b.status, Qt::CaseInsensitive);
                if (statusOrder != 0)
                    return statusOrder < 0;
                return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
            }
            return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
        });

        visibleStationIndices = matchingStations;
        table->clearContents();
        table->setRowCount(visibleStationIndices.size());
        for (int row = 0; row < visibleStationIndices.size(); ++row) {
            const int stationIndex = visibleStationIndices[row];
            const Station &station = stations[stationIndex];
            QString stationDescription = station.name + '\n' + station.type;
            QString values[] = {station.id, stationDescription, station.cityRegion,
                                QString::number(station.equipment), QString::number(station.availability) + "%",
                                station.status, station.lastUpdate};
            for (int column = 0; column < 7; ++column) {
                if (column == 4) {
                    auto *cell = new QWidget;
                    cell->setAttribute(Qt::WA_TransparentForMouseEvents);
                    auto *line = new QHBoxLayout(cell);
                    line->setContentsMargins(5, 0, 5, 0);
                    line->setSpacing(5);
                    const QColor fill = station.status == "Maintenance" ? QColor("#ffb000") :
                                        station.status == "Offline" ? QColor("#fa354b") : QColor("#00bd70");
                    line->addWidget(new Bar(station.availability, fill), 1);
                    line->addWidget(label(QString::number(station.availability) + "%", "tiny"));
                    table->setCellWidget(row, column, cell);
                    continue;
                }
                if (column == 5) {
                    auto *cell = new QWidget;
                    cell->setAttribute(Qt::WA_TransparentForMouseEvents);
                    auto *line = new QHBoxLayout(cell);
                    line->setContentsMargins(8, 0, 8, 0);
                    auto *status = new QLabel(station.status);
                    status->setAlignment(Qt::AlignCenter);
                    status->setFixedHeight(28);
                    status->setMinimumWidth(106);
                    const QString background = station.status == "Maintenance" ? "#fef3c7" :
                                               station.status == "Offline" ? "#fee2e2" : "#dcfce7";
                    const QString foreground = station.status == "Maintenance" ? "#d97706" :
                                               station.status == "Offline" ? "#ef4444" : "#16a34a";
                    status->setStyleSheet(QString("background:%1;color:%2;border-radius:14px;font-size:13px;border:2px solid %1;")
                                          .arg(background, foreground));
                    pulseStatuses.append({status,station.status});
                    line->addWidget(status);
                    line->addStretch();
                    table->setCellWidget(row, column, cell);
                    continue;
                }
                auto *item = new QTableWidgetItem(values[column]);
                item->setTextAlignment(column == 3 ? Qt::AlignCenter : Qt::AlignVCenter | Qt::AlignLeft);
                if (column == 1)
                    if (stationIndex < 8)
                        item->setIcon(QIcon(QString(":/assets/stations/station_%1.png").arg(stationIndex + 1, 2, 10, QChar('0'))));
                if (column == 6)
                    item->setForeground(QColor("#087cff"));
                table->setItem(row, column, item);
            }
        }
        const QDateTime now=QDateTime::currentDateTime();
        for(int row=0;row<table->rowCount();++row) {
            auto *item=table->item(row,6); if(!item) continue;
            const QString raw=stations[visibleStationIndices[row]].lastUpdate;
            const QDateTime updated=stationUpdateDate(raw);
            if(!updated.isValid()) { item->setForeground(QColor("#a15c00")); item->setToolTip("Last update date unavailable; freshness cannot be assessed."); }
            else if(updated.daysTo(now)>staleDays->value()) { item->setForeground(QColor("#b42318")); item->setToolTip(QString("Stale update: %1 days old. Threshold: %2 days.").arg(updated.daysTo(now)).arg(staleDays->value())); }
            else { item->setForeground(QColor("#087cff")); item->setToolTip("Last update is within the selected freshness threshold."); }
        }
        refreshMetrics();
        updateSituation();
    }

    void refreshMetrics() {
        int online = 0;
        int offline = 0;
        int totalAvailability = 0;
        for (const Station &station : stations) {
            if (station.status == "Online")
                ++online;
            if (station.status == "Offline")
                ++offline;
            totalAvailability += station.availability;
        }
        const int average = stations.isEmpty() ? 0 : qRound(double(totalAvailability) / stations.size());
        findChild<QLabel *>("totalValue")->setText(QString::number(stations.size()));
        findChild<QLabel *>("onlineValue")->setText(QString::number(online));
        findChild<QLabel *>("offlineValue")->setText(QString::number(offline));
        findChild<QLabel *>("availabilityValue")->setText(QString::number(average) + "%");
    }
};
}

void setupStationUi(QWidget *root) {
    root->setObjectName("stationRoot");
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *page = new StationPage;
    // The integrated app already provides the main navigation sidebar.
    // Keep the station page's own search bar and content, but do not nest
    // the standalone module's second navigation column inside it.
    layout->addWidget(page, 1);

    static bool soundFilterInstalled = false;
    if (!soundFilterInstalled) {
        qApp->installEventFilter(new ButtonSoundFilter(qApp));
        soundFilterInstalled = true;
    }

    QFile stylesheet(":/styles/station.qss");
    if (stylesheet.open(QIODevice::ReadOnly))
        root->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
}
