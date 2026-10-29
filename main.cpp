#include "skysensewindow.h"
#include "connection.h"
#include "apptheme.h"
#include <QApplication>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SkySense");
    app.setOrganizationName("CoreVision");
    app.setStyle("Fusion");
    AppTheme::instance()->initialize();

    // On recupere l instance unique du singleton
    Connection &c = Connection::createInstance();

    // On tente d etablir la connexion (ODBC / Oracle)
    bool test = c.createConnection();

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

    //************************Test Connection BD**************************//
    if (test) {
        QMessageBox::information(nullptr, QObject::tr("Base de donnees"),
                                 QObject::tr("Connexion reussie."), QMessageBox::Ok);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Base de donnees"),
                              QObject::tr("Echec de la connexion."), QMessageBox::Ok);
    }
    return app.exec();
}
