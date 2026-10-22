#include <QApplication>
#include <QDebug>
#include <QIcon>
#include "MainWindow.h"
#include "Database.h"
#include "Version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LaserMarkPT450");
    app.setApplicationVersion(APP_VERSION);
    app.setWindowIcon(QIcon(":/app.png"));

    if (!Database::instance().init()) {
        qDebug() << "数据库初始化失败";
    }

    MainWindow window;
    window.show();
    return app.exec();
}
