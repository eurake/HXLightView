#include "Database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

Database &Database::instance()
{
    static Database s_instance;
    return s_instance;
}

bool Database::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mydatabase.db");

    if (!db.open()) {
        qDebug() << "数据库打开失败:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS click_records ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "click_count INTEGER, "
                    "click_time TEXT)")) {
        qDebug() << "创建表失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::saveClickRecord(int count)
{
    QSqlQuery query;
    query.prepare("INSERT INTO click_records (click_count, click_time) "
                  "VALUES (:count, :time)");
    query.bindValue(":count", count);
    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    return query.exec();
}

int Database::getTotalRecords()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM click_records");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool Database::clearClickRecords(QString *errorMessage)
{
    QSqlQuery query;
    if (query.exec("DELETE FROM click_records")) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = query.lastError().text();
    }
    return false;
}
