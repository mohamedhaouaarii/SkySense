#include "apptheme.h"
#include "skysensewindow.h"
#include "connection.h"
#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QTemporaryDir>

int main(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::English,QLocale::UnitedStates));
    QApplication app(argc, argv);
    app.setApplicationName("SkySense Meriem Staff");
    app.setOrganizationName("CoreVision");
    QSettings preferences;
    if (!preferences.contains("integration/preferencesCopied")) {
        QSettings existing("CoreVision","SkySense");
        for (const QString &key:existing.allKeys())
            if (key=="appearance/darkMode" || key.startsWith("Staff/Effectifs/"))
                if (!preferences.contains(key)) preferences.setValue(key,existing.value(key));
        preferences.setValue("integration/preferencesCopied",true);
    }
    app.setStyle("Fusion");
    const bool preview = app.arguments().contains("--preview")
        || app.arguments().contains("--skip-oracle");
    app.setProperty("oraclePreview", preview);
    QTemporaryDir previewData;
    if (preview) {
        if (!previewData.isValid()) return 1;
        app.setProperty("staffStoragePath", previewData.filePath("staff.json"));
    }
    if (!app.arguments().contains("--skip-oracle")) {
        Connection &connection = Connection::createInstance();
        if (!connection.createConnection()) {
            if (!connection.lastConnectionError().isEmpty())
                QMessageBox::critical(nullptr, "Oracle connection", connection.lastConnectionError());
            return 1;
        }
    }
    AppTheme::instance()->initialize(!preview);
    SkySenseWindow window;
    const QRect available=app.primaryScreen()->availableGeometry();
    QSize targetSize(qMin(window.width(),available.width()-48),qMin(window.height(),available.height()-80));
    targetSize.setWidth(qMax(980,targetSize.width()));
    targetSize.setHeight(qMax(640,targetSize.height()));
    const QRect finalGeometry=QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,targetSize,available);
    QRect startGeometry=finalGeometry;
    startGeometry.translate(0,42);
    window.setGeometry(startGeometry);
    window.setWindowOpacity(0.0);
    window.show();
    auto *intro=new QParallelAnimationGroup(&window);
    auto *fade=new QPropertyAnimation(&window,"windowOpacity",intro);
    fade->setDuration(950); fade->setStartValue(0.0); fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    auto *slide=new QPropertyAnimation(&window,"geometry",intro);
    slide->setDuration(950); slide->setStartValue(startGeometry); slide->setEndValue(finalGeometry);
    slide->setEasingCurve(QEasingCurve::OutCubic);
    intro->start(QAbstractAnimation::DeleteWhenStopped);
    return app.exec();
}
