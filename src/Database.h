#ifndef DATABASE_H
#define DATABASE_H

#include <QString>

class Database
{
public:
    static Database &instance();

    bool init();
    bool saveClickRecord(int count);
    int getTotalRecords();
    bool clearClickRecords(QString *errorMessage = nullptr);

private:
    Database() = default;
    ~Database() = default;
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;
};

#endif // DATABASE_H
