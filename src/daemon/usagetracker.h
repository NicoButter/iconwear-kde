#ifndef USAGETRACKER_H
#define USAGETRACKER_H

#include <QObject>
#include <QMap>
#include <KActivities/Stats/ResultSet>

class UsageTracker : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.iconwear.Tracker")

public:
    explicit UsageTracker(QObject *parent = nullptr);

public Q_SLOTS:
    int getWearLevel(const QString &appId);

private Q_SLOTS:
    void onResourceOpened(const QString &activity, const QString &agent, const QString &resource);

Q_SIGNALS:
    void wearLevelChanged(const QString &appId, int newLevel);

private:
    void updateStats();
    void loadConfig();
    void saveConfig();
    QMap<QString, int> m_wearLevels;
};

#endif // USAGETRACKER_H
