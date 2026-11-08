#include "boot_manager/gui/main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral(BOOT_MANAGER_VERSION));
    MainWindow window;
    window.show();
    return app.exec();
}
