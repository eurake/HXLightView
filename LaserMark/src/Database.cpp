#include "Database.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

Database &Database::instance()
{
    static Database s_instance;
    return s_instance;
}

bool Database::init()
{
    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("lasermark.db");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "数据库打开失败:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS mark_records ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "code TEXT, "
                    "created_time TEXT)")) {
        qDebug() << "创建表失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::saveMark(const QString &code)
{
    QSqlQuery query;
    query.prepare("INSERT INTO mark_records (code, created_time) VALUES (:code, :time)");
    query.bindValue(":code", code);
    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    return query.exec();
}

int Database::totalRecords()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM mark_records");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QList<MarkRecord> Database::recentRecords(int limit)
{
    QList<MarkRecord> records;
    QSqlQuery query;
    if (!query.exec(QString("SELECT id, code, created_time FROM mark_records "
                            "ORDER BY id DESC LIMIT %1")
                        .arg(limit))) {
        return records;
    }

    while (query.next()) {
        MarkRecord record;
        record.id = query.value(0).toInt();
        record.code = query.value(1).toString();
        record.time = query.value(2).toString();
        records.append(record);
    }
    return records;
}

bool Database::clearRecords(QString *errorMessage)
{
    QSqlQuery query;
    if (query.exec("DELETE FROM mark_records")) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = query.lastError().text();
    }
    return false;
}
