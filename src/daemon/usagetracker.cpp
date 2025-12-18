#include "usagetracker.h"
#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/Query>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

using namespace KActivities::Stats;

UsageTracker::UsageTracker(QObject *parent) : QObject(parent)
{
    loadConfig();
    updateStats();

    // Listen to ResourceOpened signal from ActivityManager
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.kde.ActivityManager"),
        QStringLiteral("/Resources"),
        QStringLiteral("org.kde.ActivityManager.Resources"),
        QStringLiteral("ResourceOpened"),
        this, SLOT(onResourceOpened(QString, QString, QString))
    );
}

void UsageTracker::onResourceOpened(const QString &activity, const QString &agent, const QString &resource)
{
    Q_UNUSED(activity);
    qDebug() << "Resource opened:" << resource << "by agent:" << agent;
    
    QString appId = resource;
    if (resource.contains(QLatin1Char('/'))) {
        appId = resource.section(QLatin1Char('/'), -1);
    }

    // Increment wear level on each "click" (launch)
    m_wearLevels[appId] = qMin(100, m_wearLevels.value(appId, 0) + 1);
    saveConfig();
    Q_EMIT wearLevelChanged(appId, m_wearLevels[appId]);
}

void UsageTracker::loadConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("iconwearrc")), QStringLiteral("WearLevels"));
    const QStringList keys = config.keyList();
    for (const QString &key : keys) {
        m_wearLevels[key] = config.readEntry(key, 0);
    }
}

void UsageTracker::saveConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("iconwearrc")), QStringLiteral("WearLevels"));
    QMapIterator<QString, int> i(m_wearLevels);
    while (i.hasNext()) {
        i.next();
        config.writeEntry(i.key(), i.value());
    }
    config.sync();
}

int UsageTracker::getWearLevel(const QString &appId)
{
    // appId could be a desktop file name like "org.kde.dolphin.desktop"
    return m_wearLevels.value(appId, 0);
}

void UsageTracker::updateStats()
{
    // Query for frequently used applications
    Query query;
    query.setAgents(QStringList{QStringLiteral("plasma-desktop")});
    query.setOrdering(Terms::HighScoredFirst);

    ResultSet results(query);

    for (const ResultSet::Result &result : results) {
        QString resource = result.resource(); // This is often the desktop file path or URL
        double score = result.score();
        
        // Map score to 0-100 wear level
        // This is a placeholder mapping logic
        int wearLevel = qMin(100, static_cast<int>(score * 10)); 
        
        QString appId = resource;
        if (resource.contains(QLatin1Char('/'))) {
            appId = resource.section(QLatin1Char('/'), -1);
        }

        if (m_wearLevels[appId] != wearLevel) {
            m_wearLevels[appId] = wearLevel;
            Q_EMIT wearLevelChanged(appId, wearLevel);
        }
    }
}
