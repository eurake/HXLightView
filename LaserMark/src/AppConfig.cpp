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
    result.prefix = settings.value("code/prefix", result.prefix).toString();
    result.number = settings.value("code/number", result.number).toInt();
    result.width = settings.value("code/width", result.width).toInt();

    if (result.host.isEmpty()) {
        result.host = "127.0.0.1";
    }
    if (result.port < 1 || result.port > 65535) {
        result.port = 9999;
    }
    if (result.number < 0) {
        result.number = 1;
    }
    if (result.width < 1 || result.width > 12) {
        result.width = 4;
    }
    return result;
}

void AppConfig::save(const MarkSettings &settings)
{
    QSettings ini(filePath(), QSettings::IniFormat);
    ini.setIniCodec("UTF-8");
    ini.setValue("server/host", settings.host);
    ini.setValue("server/port", settings.port);
    ini.setValue("code/prefix", settings.prefix);
    ini.setValue("code/number", settings.number);
    ini.setValue("code/width", settings.width);
    ini.sync();
}
