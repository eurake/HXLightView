#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

struct MarkSettings
{
    QString host = "127.0.0.1";
    int port = 9999;
    QString batch;
    QString date;
    QString serialPort;
    int baud = 9600;
};

class AppConfig
{
public:
    static QString filePath();
    static MarkSettings load();
    static void save(const MarkSettings &settings);
};

#endif // APPCONFIG_H
