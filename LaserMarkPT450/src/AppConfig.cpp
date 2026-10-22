#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

QString AppConfig::filePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("config.ini");
}

MarkSettings AppConfig::load()
{
    QSettings settings(filePath(), QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    MarkSettings result;
    result.host = settings.value("server/host", result.host).toString();
    result.port = settings.value("server/port", result.port).toInt();
    result.batch = settings.value("code/batch", result.batch).toString();
    result.date = settings.value("code/date", result.date).toString();
    result.serialPort = settings.value("serial/port", result.serialPort).toString();
    result.baud = settings.value("serial/baud", result.baud).toInt();

    if (result.host.isEmpty()) {
        result.host = "127.0.0.1";
    }
    if (result.port < 1 || result.port > 65535) {
        result.port = 9999;
    }
    const int knownBauds[] = {9600, 19200, 38400, 57600, 115200};
    bool baudOk = false;
    for (int baud : knownBauds) {
        if (result.baud == baud) {
            baudOk = true;
            break;
        }
    }
    if (!baudOk) {
        result.baud = 9600;
    }
    return result;
}

void AppConfig::save(const MarkSettings &settings)
{
    QSettings ini(filePath(), QSettings::IniFormat);
    ini.setIniCodec("UTF-8");
    ini.setValue("server/host", settings.host);
    ini.setValue("server/port", settings.port);
    ini.setValue("code/batch", settings.batch);
    ini.setValue("code/date", settings.date);
    ini.setValue("serial/port", settings.serialPort);
    ini.setValue("serial/baud", settings.baud);
    ini.sync();
}
