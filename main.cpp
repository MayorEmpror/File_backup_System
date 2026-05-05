#include <QApplication>
#include <QFont>
#include "./src/UI/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont uiFont("Segoe UI", 10);
    if (!uiFont.exactMatch()) {
        uiFont = QFont("SF Pro Text", 10);
    }
    if (!uiFont.exactMatch()) {
        uiFont = QFont();
        uiFont.setPointSize(10);
    }
    app.setFont(uiFont);

    MainWindow window;
    window.setMinimumSize(880, 560);
    window.resize(1280, 820);
    window.show();

    return app.exec();
}