#include "skysensewindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SkySense");
    app.setOrganizationName("CoreVision");
    app.setStyle("Fusion");
    SkySenseWindow window;
    window.show();
    return app.exec();
}
