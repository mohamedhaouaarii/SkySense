#include "equipmentdata.h"

#include <QDateTime>
#include <QFileInfo>
#include <QPainter>
#include <QPageLayout>
#include <QPageSize>
#include <QPixmap>
#include <QImage>
#include <QPdfWriter>
#include <QSaveFile>
#include <algorithm>

namespace {
const QColor navy("#091B40"), blue("#0785FF"), muted("#5D77AD"), border("#D9E6F5");

QString stationName(const EquipmentRecord &record) {
    return record.station.section('\n', 0, 0).trimmed().isEmpty() ? "Reserve / non affecte" : record.station.section('\n', 0, 0).trimmed();
}
void font(QPainter &p, int size, QColor color = navy, int weight = QFont::Normal) {
    p.setFont(QFont("Arial", size, weight)); p.setPen(color);
}
void header(QPainter &p, int w, int page, int pages, const QString &title) {
    p.fillRect(0, 0, w, 8, blue);
    font(p, 21, navy, QFont::Bold); p.drawText(QRect(36, 24, w-72, 34), Qt::AlignVCenter, "SkySense | " + title);
    p.setPen(border); p.drawLine(36, 68, w-36, 68);
    font(p, 8, muted); p.drawText(QRect(36, 760, w-72, 18), Qt::AlignVCenter, QString("Inventaire equipement  |  Page %1 / %2").arg(page).arg(pages));
}
QString dateLabel(const EquipmentRecord &r) {
    const auto dt = QDateTime::fromString(r.updated, Qt::ISODate);
    return dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : "Non renseignee";
}
QPixmap reportPhoto(const EquipmentRecord &record, const QSize &size) {
    QPixmap photo;
    if (!record.photoBase64.isEmpty())
        photo.loadFromData(QByteArray::fromBase64(record.photoBase64.toLatin1()), "PNG");
    if (!photo.isNull())
        return photo.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap placeholder(size);
    placeholder.fill(QColor("#EAF5FF"));
    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#D5ECFF"));
    painter.drawEllipse(QRectF(size.width() * .2, size.height() * .12,
                               size.width() * .6, size.height() * .76));
    font(painter, qMax(8, size.height() / 5), blue, QFont::Bold);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, record.type.left(2).toUpper());
    return placeholder;
}
void drawReportPhoto(QPainter &painter, const EquipmentRecord &record, const QRect &target) {
    painter.save();
    painter.setClipRect(target);
    painter.fillRect(target, QColor("#EAF5FF"));
    const QPixmap photo = reportPhoto(record, target.size());
    const QRect destination(target.x() + (target.width() - photo.width()) / 2,
                            target.y() + (target.height() - photo.height()) / 2,
                            photo.width(), photo.height());
    painter.drawPixmap(destination, photo, photo.rect());
    painter.restore();
}
void drawStatus(QPainter &painter, const QRect &rect, const QString &status) {
    const QColor color = status == "Online" ? QColor("#15803D")
                        : status == "Offline" ? QColor("#DC2626") : QColor("#B45309");
    const QColor fill = status == "Online" ? QColor("#DCFCE7")
                       : status == "Offline" ? QColor("#FEE2E2") : QColor("#FEF3C7");
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawRoundedRect(rect, rect.height() / 2, rect.height() / 2);
    font(painter, 8, color, QFont::DemiBold);
    painter.drawText(rect, Qt::AlignCenter, status);
}
}

bool exportEquipmentPdf(const QString &filePath, const QVector<EquipmentRecord> &records,
                        const QDate &reportDate, QString *error, bool selectedOnly) {
    auto fail = [error](const QString &message) { if (error) *error = message; return false; };
    if (filePath.trimmed().isEmpty()) return fail("Chemin du PDF vide.");
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    QPdfWriter writer(&file);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(10, 10, 10, 10));
    writer.setResolution(96);
    writer.setTitle("SkySense - Inventaire des equipements");
    QPainter p;
    if (!p.begin(&writer)) return fail("Impossible de generer le PDF.");
    const int w = writer.width(), h = writer.height();
    const int rowsPerPage = 12;
    const int pages = qMax(1, (records.size() + rowsPerPage - 1) / rowsPerPage);
    for (int page = 0; page < pages; ++page) {
        if (page && !writer.newPage()) { p.end(); return fail("Impossible de creer une page PDF."); }
        p.fillRect(0, 0, w, h, Qt::white);
        header(p, w, page + 1, pages, selectedOnly ? "Selection d'equipements" : "Inventaire complet");
        if (page == 0) {
            font(p, 11, muted); p.drawText(QRect(36, 76, w-72, 24), Qt::AlignVCenter,
                QString("Document du %1  |  %2 equipement(s)").arg(reportDate.toString("dd/MM/yyyy")).arg(records.size()));
        }
        const int top = 112;
        const qreal availableWidth = w - 72;
        const qreal baseWidths[] = {62, 96, 128, 138, 142, 104, 58, 82, 145};
        const qreal baseTotal = 955;
        qreal widths[9];
        for (int i = 0; i < 9; ++i) widths[i] = baseWidths[i] * availableWidth / baseTotal;
        const QStringList labels{"Photo", "ID / Type", "Modele", "Numero de serie", "Station", "Emplacement", "Unite", "Statut", "Derniere mise a jour"};
        int x = 36;
        p.setPen(Qt::NoPen); p.setBrush(navy); p.drawRect(36, top, w-72, 30);
        font(p, 8, Qt::white, QFont::DemiBold);
        for (int c=0; c<9; ++c) { p.drawText(QRect(x+4, top, qMax(1, int(widths[c])-8), 30), Qt::AlignVCenter, labels[c]); x += qRound(widths[c]); }
        const int start = page * rowsPerPage, end = qMin(start + rowsPerPage, records.size());
        for (int i=start; i<end; ++i) {
            const auto &r = records[i]; const int y = top + 30 + (i-start)*45;
            p.fillRect(36, y, w-72, 45, ((i-start)%2) ? QColor("#F4F8FD") : Qt::white);
            p.setPen(border); p.drawLine(36, y+45, w-36, y+45); x=36;
            const QStringList vals{QString(), r.id + "\n" + r.type, r.model, r.serialNumber,
                stationName(r), r.location.trimmed().isEmpty() ? "Non renseigne" : r.location,
                r.unit, r.status, dateLabel(r)};
            for (int c=0; c<9; ++c) {
                const int columnWidth = qRound(widths[c]);
                if (c == 0) drawReportPhoto(p, r, QRect(x+5, y+5, qMax(1,columnWidth-10), 35));
                else if (c == 7) drawStatus(p, QRect(x+4,y+11,qMax(1,columnWidth-8),23), vals[c]);
                else { font(p, 8, navy); p.drawText(QRect(x+5, y+3, qMax(1,columnWidth-10), 39), Qt::AlignVCenter|Qt::TextWordWrap, vals[c]); }
                x += columnWidth;
            }
        }
        if (records.isEmpty()) { font(p, 13, muted); p.drawText(QRect(36, 190, w-72, 50), Qt::AlignCenter, "Aucun equipement dans cette selection."); }
        font(p, 8, muted); p.drawText(QRect(36, h-48, w-72, 20), Qt::AlignVCenter,
            "Donnees de gestion de l'equipement. Les mesures meteo ne sont pas gerees dans ce module.");
    }
    // Technical profiles are included after the inventory, with recommendation dates/status.
    for (int i=0; i<records.size(); ++i) {
        if (!writer.newPage()) { p.end(); return fail("Impossible de creer une page de profil PDF."); }
        const int page = pages + i + 1;
        p.fillRect(0,0,w,h,Qt::white); header(p,w,page,pages+records.size(),"Fiche equipement");
        const auto &r = records[i]; const auto assessment = assessEquipment(r);
        font(p, 20, navy, QFont::Bold); p.drawText(QRect(42, 92, w-300, 34), Qt::AlignVCenter, r.id + " - " + r.type);
        font(p, 11, muted); p.drawText(QRect(42, 130, w-300, 24), Qt::AlignVCenter, r.model + "  |  Serie : " + r.serialNumber);
        drawReportPhoto(p, r, QRect(w-210, 92, 160, 110));
        const QStringList details{"Modele : " + r.model, "Numero de serie : " + r.serialNumber,
            "Station : " + stationName(r), "Emplacement : " + (r.location.isEmpty() ? "Non renseigne" : r.location),
            "Unite : " + r.unit, "Statut : " + r.status,
            "Derniere mise a jour : " + dateLabel(r), "Fraicheur : " + assessment.freshness};
        int y=220;
        for (const auto &line : details) { p.setPen(border); p.drawLine(42,y+34,w-42,y+34); font(p,11,navy); p.drawText(QRect(52,y,w-104,34),Qt::AlignVCenter,line); y+=38; }
        p.setPen(Qt::NoPen); p.setBrush(assessment.needsReview ? QColor("#FFF6DF") : QColor("#EAF8EF")); p.drawRoundedRect(QRect(42,y+20,w-84,88),10,10);
        font(p,12,navy,QFont::DemiBold); p.drawText(QRect(58,y+29,w-116,24),Qt::AlignVCenter,"Evaluation et recommandation");
        font(p,10,muted); p.drawText(QRect(58,y+54,w-116,45),Qt::TextWordWrap,assessment.recommendation);
    }
    p.end();
    if (!file.commit()) return fail(file.errorString());
    if (error) error->clear();
    return true;
}
