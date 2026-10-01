#include "skysensewindow.h"
#include <QApplication>
#include <QTimer>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SkySense Staff");
    app.setOrganizationName("CoreVision");
    app.setStyle("Fusion");
    SkySenseWindow window;
    const QStringList arguments = app.arguments();
    const int capture = arguments.indexOf("--screenshot");
    if (capture >= 0 && capture + 1 < arguments.size()) {
        // Render the real Qt window at the original project's dimensions, without OS chrome.
        window.setAttribute(Qt::WA_DontShowOnScreen);
        window.resize(1646, 932);
        window.show();
        QTimer::singleShot(300, [&] {
            QImage image = window.centralWidget()->grab().toImage();
            image.setDevicePixelRatio(1);
            image = image.scaled(window.centralWidget()->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            const bool saved = image.save(arguments[capture + 1]);
            app.exit(saved ? 0 : 1);
        });
    } else {
        window.show();
    }
    return app.exec();
}
