#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QList>

struct MarkRecord
{
    int id = 0;
    QString code;
    QString time;
};

class Database
{
public:
    static Database &instance();

    bool init();
    bool saveMark(const QString &code);
    int totalRecords();
    QList<MarkRecord> recentRecords(int limit = 50);
    bool clearRecords(QString *errorMessage = nullptr);

private:
    Database() = default;
    ~Database() = default;
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;
};

#endif // DATABASE_H
