#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include "usagetracker.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("iconwear-daemon"));
    app.setOrganizationDomain(QStringLiteral("org.kde"));

    UsageTracker tracker;

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.iconwear"))) {
        qCritical() << "Failed to register DBus service org.kde.iconwear";
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Tracker"), &tracker, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qCritical() << "Failed to register DBus object /Tracker";
        return 1;
    }

    qInfo() << "IconWear Daemon started...";

    return app.exec();
}
