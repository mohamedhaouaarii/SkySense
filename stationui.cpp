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
#include <QPainter>
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
#include <QStringList>
#include <QTableWidget>
#include <QTime>
#include <QtMath>
#include <QVBoxLayout>
#include <QVector>
#include <QUrl>
#include <algorithm>
#include <functional>

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

QPushButton *button(const QString &text, bool primary = false) {
    auto *widget = new QPushButton(text);
    widget->setProperty("primary", primary);
    widget->setMinimumHeight(36);
    widget->setCursor(Qt::PointingHandCursor);
    return widget;
}

QLineEdit *searchBox(const QString &hint) {
    auto *widget = new QLineEdit;
    widget->setPlaceholderText(QString::fromUtf8("⌕  ") + hint);
    widget->setMinimumHeight(34);
    return widget;
}

QComboBox *filter(const QString &defaultText, const QStringList &options = {}) {
    auto *widget = new QComboBox;
    widget->addItem(defaultText, QString());
    for (const QString &option : options)
        widget->addItem(option, option);
    widget->setMinimumHeight(34);
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
        brand->setPixmap(QPixmap(":/assets/stations/station_logo.png").scaled(
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

class MapPanel final : public QWidget {
    QPixmap mapImage;
public:
    MapPanel() : mapImage(":/assets/stations/network_map.png") {
        setMinimumSize(180, 96);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(rect(), QColor("#ffffff"));
        if (mapImage.isNull())
            return;

        const QRect bounds = rect().adjusted(4, 4, -4, -4);
        QSize fittedSize = mapImage.size();
        fittedSize.scale(bounds.size(), Qt::KeepAspectRatio);
        const QRect imageRect(bounds.left() + (bounds.width() - fittedSize.width()) / 2,
                              bounds.top() + (bounds.height() - fittedSize.height()) / 2,
                              fittedSize.width(), fittedSize.height());
        painter.drawPixmap(imageRect, mapImage, mapImage.rect());

        // Restore the marker clipped by the left edge of the supplied bitmap.
        const qreal scale = imageRect.width() / qreal(mapImage.width());
        const QPointF markerCenter(imageRect.left() + 15.0 * scale,
                                   imageRect.top() + 104.0 * scale);
        const qreal radius = 13.0 * scale;
        painter.setPen(QPen(Qt::white, qMax(1.0, 3.0 * scale)));
        painter.setBrush(QColor("#00bd70"));
        painter.drawEllipse(markerCenter, radius, radius);
    }
};

QWidget *metricCard(const QString &title, const QString &value, const QString &tail,
                    const QString &trend, const QString &tone, const QString &glyph,
                    const QString &valueObjectName) {
    auto *frame = card("metric");
    frame->setProperty("tone", tone);
    auto *horizontal = new QHBoxLayout(frame);
    horizontal->setContentsMargins(10, 8, 8, 7);
    horizontal->setSpacing(7);
    auto *bubble = label(glyph, "metricGlyph");
    bubble->setProperty("tone", tone);
    bubble->setAlignment(Qt::AlignCenter);
    bubble->setFixedSize(32, 32);
    horizontal->addWidget(bubble, 0, Qt::AlignTop);

    auto *vertical = new QVBoxLayout;
    vertical->setSpacing(0);
    vertical->addWidget(label(title, "metricTitle"));
    auto *numberLine = new QHBoxLayout;
    numberLine->setSpacing(4);
    auto *number = label(value, "number");
    number->setObjectName(valueObjectName);
    numberLine->addWidget(number);
    numberLine->addWidget(label(tail, "muted"), 0, Qt::AlignBottom);
    vertical->addLayout(numberLine);
    vertical->addWidget(label(trend, "trend"));
    horizontal->addLayout(vertical, 1);
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
    const QStringList regions{"Rocky Mountains", "Pacific Coast", "Great Plains", "Southwest",
                              "Sierra Nevada", "Alaska"};
    const QStringList counts{"5 / 5 stations", "4 / 5 stations", "4 / 5 stations",
                             "3 / 4 stations", "4 / 4 stations", "2 / 3 stations"};
    const int values[] = {100, 80, 80, 75, 100, 67};
    for (int index = 0; index < regions.size(); ++index) {
        auto *region = card("region");
        region->setMaximumHeight(70);
        auto *content = new QVBoxLayout(region);
        content->setContentsMargins(6, 5, 6, 5);
        content->setSpacing(1);
        content->addWidget(label(regions[index], "regionName"));
        content->addWidget(label(counts[index], "regionCount"));
        auto *line = new QHBoxLayout;
        line->addWidget(new Bar(values[index], index == 3 ? QColor("#ffb000") :
                                index == 5 ? QColor("#fa354b") : QColor("#00bd70")), 1);
        line->addWidget(label(QString::number(values[index]) + "%", "tiny"));
        content->addLayout(line);
        row->addWidget(region, 1);
    }
    vertical->addLayout(row);
    return frame;
}

struct Station {
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
            {"ALP-03", "Alpine Ridge", "Mountain Station", "Aspen, CO\nRocky Mountains", 8, 100, "Online", "2 min ago"},
            {"COA-01", "Coastal Bay", "Coastal Station", "Santa Cruz, CA\nPacific Coast", 6, 100, "Online", "4 min ago"},
            {"FOR-12", "Pine Forest", "Forest Station", "Bend, OR\nCascade Range", 7, 86, "Online", "20 min ago"},
            {"DES-05", "Desert View", "Desert Station", "Phoenix, AZ\nSonoran Desert", 5, 60, "Maintenance", "1 hour ago"},
            {"LAK-04", "Lake Valley", "Lake Station", "Lake Tahoe, CA\nSierra Nevada", 9, 100, "Online", "3 min ago"},
            {"PLA-08", "Great Plains", "Plains Station", "Dodge City, KS\nGreat Plains", 6, 96, "Online", "12 min ago"},
            {"VAL-07", "Green Valley", "Valley Station", "Boise, ID\nTreasure Valley", 9, 22, "Offline", "2 hours ago"},
            {"NOR-11", "Northern Peak", "Arctic Station", "Fairbanks, AK\nInterior Alaska", 7, 100, "Online", "6 min ago"}
        };

        auto *outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->setSpacing(0);

        auto *topBar = card("topbar");
        auto *topLayout = new QHBoxLayout(topBar);
        topLayout->setContentsMargins(16, 6, 15, 6);
        globalSearch = searchBox("Search for a station, city, or region…");
        topLayout->addWidget(globalSearch, 1);
        topLayout->addStretch();
        QString date = QLocale(QLocale::English, QLocale::UnitedStates).toString(QDate::currentDate(), "ddd, MMM d, yyyy");
        if (!date.isEmpty())
            date[0] = date[0].toUpper();
        topLayout->addWidget(label(QString::fromUtf8("▣   ") + date, "topText"));
        topLayout->addSpacing(8);
        topLayout->addWidget(label(QString::fromUtf8("👤  Alex Martin\n      Operations Manager   ⌄"), "user"));
        outer->addWidget(topBar);

        scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        outer->addWidget(scrollArea, 1);

        auto *content = new QWidget;
        auto *body = new QVBoxLayout(content);
        body->setContentsMargins(16, 8, 14, 10);
        body->setSpacing(7);

        auto *heading = new QHBoxLayout;
        auto *copy = new QVBoxLayout;
        copy->setSpacing(0);
        copy->addWidget(label("Stations", "pageTitle"));
        auto *subtitle = label("Track station availability, location, and network performance.", "subtitle");
        subtitle->setWordWrap(true);
        copy->addWidget(subtitle);
        heading->addLayout(copy, 1);
        body->addLayout(heading);

        auto *actions = new QHBoxLayout;
        actions->addStretch();
        auto *addButton = button(QString::fromUtf8("＋  Add Station"), true);
        auto *editButton = button("Edit Station");
        auto *deleteButton = button("Delete Station");
        auto *mapButton = button(QString::fromUtf8("⌖  Map"));
        auto *exportButton = button(QString::fromUtf8("⇩  Export PDF"), true);
        actions->addWidget(addButton);
        actions->addWidget(editButton);
        actions->addWidget(deleteButton);
        actions->addWidget(mapButton);
        actions->addWidget(exportButton);
        body->addLayout(actions);

        auto *metrics = new QHBoxLayout;
        metrics->setSpacing(9);
        metrics->addWidget(metricCard("Total Stations", "0", "in total", "In this list", "blue", "●", "totalValue"), 1);
        metrics->addWidget(metricCard("Online Stations", "0", "active", "Current status", "green", "◉", "onlineValue"), 1);
        metrics->addWidget(metricCard("Offline Stations", "0", "to check", "Current status", "red", "△", "offlineValue"), 1);
        metrics->addWidget(metricCard("Average Availability", "0%", "of the network", "Based on listed stations", "purple", "●", "availabilityValue"), 1);
        body->addLayout(metrics);

        auto *filters = new QHBoxLayout;
        filters->setSpacing(7);
        tableSearch = searchBox("Search by ID, name, city, or region…");
        regionFilter = filter("All Regions");
        statusFilter = filter("All Statuses", {"Online", "Maintenance", "Offline"});
        typeFilter = filter("All Station Types");
        sortFilter = new QComboBox;
        sortFilter->addItem("Sort by: Name", "name");
        sortFilter->addItem("Sort by: ID", "id");
        sortFilter->addItem("Sort by: Availability", "availability");
        for (QComboBox *combo : {regionFilter, statusFilter, typeFilter, sortFilter})
            combo->setMinimumHeight(38);
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
        table->horizontalHeader()->setFixedHeight(28);
        table->verticalHeader()->setDefaultSectionSize(46);
        table->setIconSize(QSize(30, 20));
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setShowGrid(false);
        table->setAlternatingRowColors(false);
        table->setMinimumHeight(28 + 8 * 46 + 4);
        body->addWidget(table);

        auto *bottomSummary = new QHBoxLayout;
        bottomSummary->setSpacing(8);
        bottomSummary->addWidget(regional(), 7);
        auto *mapCard = card();
        mapCard->setFixedHeight(150);
        auto *mapLayout = new QVBoxLayout(mapCard);
        mapLayout->setContentsMargins(9, 7, 9, 7);
        mapLayout->setSpacing(4);
        mapLayout->addWidget(label("Station Network Map", "mapTitle"));
        mapLayout->addWidget(new MapPanel, 1);
        bottomSummary->addWidget(mapCard, 3);
        body->addLayout(bottomSummary);
        scrollArea->setWidget(content);

        refreshFilterOptions();
        QObject::connect(globalSearch, &QLineEdit::textChanged, this, [this] { refreshTable(); });
        QObject::connect(tableSearch, &QLineEdit::textChanged, this, [this] { refreshTable(); });
        QObject::connect(regionFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(statusFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(typeFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(sortFilter, &QComboBox::currentTextChanged, this, [this] { refreshTable(); });
        QObject::connect(addButton, &QPushButton::clicked, this, [this] { addStation(); });
        QObject::connect(editButton, &QPushButton::clicked, this, [this] { editStation(); });
        QObject::connect(deleteButton, &QPushButton::clicked, this, [this] { deleteStation(); });
        QObject::connect(mapButton, &QPushButton::clicked, this, [this] { showMap(); });
        QObject::connect(exportButton, &QPushButton::clicked, this, [this] { exportPdf(); });
        refreshTable();
    }

    void showStations() {
        scrollArea->verticalScrollBar()->setValue(0);
    }

    void showMap() {
        QDialog dialog(this);
        dialog.setWindowTitle("Station Network Map");
        dialog.setMinimumSize(760, 480);
        auto *layout = new QVBoxLayout(&dialog);
        layout->addWidget(label("Weather Station Locations", "pageTitle"));
        auto *map = new MapPanel;
        map->setMinimumSize(700, 370);
        layout->addWidget(map, 1);
        layout->addWidget(label(QString::number(stations.size()) + " stations in this list", "subtitle"));
        auto *close = button("Close");
        auto *buttons = new QHBoxLayout;
        buttons->addStretch();
        buttons->addWidget(close);
        layout->addLayout(buttons);
        QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
        dialog.exec();
    }

private:
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
            QMessageBox::warning(this, "PDF Export", "The PDF file could not be created.");
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
        QMessageBox::information(this, "PDF Export", "The report was saved to:\n" + fileName);
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
            QMessageBox::information(this, action, "Select a station from the table first.");
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
        auto *equipment = new QSpinBox(&dialog);
        equipment->setRange(0, 999);
        auto *availability = new QSpinBox(&dialog);
        availability->setRange(0, 100);
        availability->setSuffix(" %");
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
            equipment->setValue(station.equipment);
            availability->setValue(station.availability);
            status->setCurrentText(station.status);
        }

        form->addRow("Station ID:", id);
        form->addRow("Name:", name);
        form->addRow("Station Type:", type);
        form->addRow("City:", city);
        form->addRow("Region:", region);
        form->addRow("Equipment Count:", equipment);
        form->addRow("Availability:", availability);
        form->addRow("Status:", status);
        layout->addLayout(form);

        auto *actions = new QHBoxLayout;
        actions->addStretch();
        auto *cancel = button("Cancel");
        auto *save = button(editing ? "Save" : "Add", true);
        actions->addWidget(cancel);
        actions->addWidget(save);
        layout->addLayout(actions);

        QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(save, &QPushButton::clicked, &dialog, [&] {
            const QString cleanId = id->text().trimmed();
            const QString cleanName = name->text().trimmed();
            if (cleanId.isEmpty() || cleanName.isEmpty()) {
                QMessageBox::warning(&dialog, "Required Fields", "The station ID and name are required.");
                return;
            }
            for (int index = 0; index < stations.size(); ++index) {
                if (index != row && stations[index].id.compare(cleanId, Qt::CaseInsensitive) == 0) {
                    QMessageBox::warning(&dialog, "ID Already Used", "A station already uses this ID.");
                    return;
                }
            }
            dialog.accept();
        });

        if (dialog.exec() != QDialog::Accepted)
            return;

        Station station;
        station.id = id->text().trimmed();
        station.name = name->text().trimmed();
        station.type = type->text().trimmed();
        station.cityRegion = city->text().trimmed();
        if (!region->text().trimmed().isEmpty())
            station.cityRegion += '\n' + region->text().trimmed();
        station.equipment = equipment->value();
        station.availability = availability->value();
        station.status = status->currentText();
        station.lastUpdate = "Just now, " + currentTime();

        const int changedStationIndex = editing ? row : stations.size();
        if (editing)
            stations[row] = station;
        else
            stations.append(station);
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
        auto *cancelButton = confirmation.addButton("Cancel", QMessageBox::RejectRole);
        auto *deleteButton = confirmation.addButton("Delete", QMessageBox::DestructiveRole);
        confirmation.setDefaultButton(cancelButton);
        confirmation.exec();
        if (confirmation.clickedButton() != deleteButton)
            return;

        stations.removeAt(row);
        refreshFilterOptions();
        refreshTable();
    }

    void refreshTable() {
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
                auto *item = new QTableWidgetItem(values[column]);
                item->setTextAlignment(column == 3 ? Qt::AlignCenter : Qt::AlignVCenter | Qt::AlignLeft);
                if (column == 1)
                    if (stationIndex < 8)
                        item->setIcon(QIcon(QString(":/assets/stations/station_%1.png").arg(stationIndex + 1, 2, 10, QChar('0'))));
                if (column == 5) {
                    const QColor statusColor = station.status == "Maintenance" ? QColor("#d58b00") :
                                               station.status == "Offline" ? QColor("#e34754") : QColor("#00a866");
                    item->setForeground(statusColor);
                }
                if (column == 6)
                    item->setForeground(QColor("#087cff"));
                table->setItem(row, column, item);
            }
        }
        refreshMetrics();
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
    auto *layout = new QHBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *page = new StationPage;
    auto *sidebar = new Sidebar;
    layout->addWidget(sidebar);
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
