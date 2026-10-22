#include <QApplication>
#include <QDebug>
#include <QIcon>
#include "MainWindow.h"
#include "Database.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/app.png"));

    if (!Database::instance().init()) {
        qDebug() << "数据库初始化失败";
    }

    MainWindow window;
    window.show();
    return app.exec();
}
