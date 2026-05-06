#include <QApplication>
#include <QFont>
#include "./src/UI/MainWindow.h"

int main(int argc, char *argv[]) {
    // Create the Qt application object, passing through CLI arguments for Qt to parse (e.g. platform, style, etc.).
    QApplication app(argc, argv);

    // Choose a UI font (preferred: Segoe UI, common on Windows).
    QFont uiFont("Segoe UI", 10);
    // If the preferred font isn't available on this system, fall back to a common macOS font.
    if (!uiFont.exactMatch()) {
        uiFont = QFont("SF Pro Text", 10);
    }
    // If that still doesn't resolve to an installed font, use Qt's default font and enforce the point size.
    if (!uiFont.exactMatch()) {
        uiFont = QFont();
        uiFont.setPointSize(10);
    }
    // Apply the selected font to the entire application so widgets inherit it by default.
    app.setFont(uiFont);

    // Instantiate the main UI window (top-level widget) for the application.
    MainWindow window;
    // Set a minimum size to keep the dashboard layout usable.
    window.setMinimumSize(880, 560);
    // Provide an initial default size when the window first appears.
    window.resize(1280, 820);
    // Make the window visible on screen.
    window.show();

    // Enter Qt's event loop; this blocks until the application quits and returns the exit code.
    return app.exec();
}