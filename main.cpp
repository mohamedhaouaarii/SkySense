#include "skysensewindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Smart Weather");
    app.setOrganizationName("SkySense");
    app.setStyle("Fusion");
    SkySenseWindow window;
    window.show();
    return app.exec();
}

