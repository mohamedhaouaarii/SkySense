# SkySense — Qt Widgets

This is a native C++ Qt Widgets project, organized in the same style as the supplied Equipment example: a Qt Designer main window, a separate page builder, a shared QSS stylesheet, reusable painted icons, and Qt resource files.

## Open and run

1. Open `SkySense.pro` in Qt Creator.
2. Choose a Qt 6 Desktop kit with a matching C++ compiler.
3. Click **Configure Project**, then press **Run**.

The app starts on the Services page. Click **Add Service** to open the service form; valid submissions are added to the service table. The supplied SkySense logo and cropped Alpine clouds/mountain background are bundled in `assets/`.

## Project files

- `SkySense.pro` — qmake project.
- `skysensewindow.ui` — Qt Designer main window.
- `serviceui.cpp` and `serviceui.h` — Services page and custom UI elements.
- `styles/services.qss` — fonts, colors, cards, buttons, and navigation styling.
- `resources.qrc` — embeds the stylesheet and images in the application.
