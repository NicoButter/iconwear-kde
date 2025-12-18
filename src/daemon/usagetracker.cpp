/**
 * @file usagetracker.cpp
 * @brief Implementación del rastreador de uso de aplicaciones
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * 
 * Implementación del servicio DBus que rastrea y gestiona el desgaste
 * visual de iconos basado en patrones de uso de aplicaciones.
 */

#include "usagetracker.h"
#include <KActivities/Stats/ResultModel>
#include <KActivities/Stats/Query>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

using namespace KActivities::Stats;

//! Inicialización del servicio de rastreo de aplicaciones
UsageTracker::UsageTracker(QObject *parent) : QObject(parent)
{
    loadConfig();
    updateStats();

    // Timer para verificar aplicaciones activas cada 30 segundos
    m_activityCheckTimer = new QTimer(this);
    connect(m_activityCheckTimer, &QTimer::timeout, this, &UsageTracker::checkActiveApplications);
    m_activityCheckTimer->start(30000);  // Cada 30 segundos

    // Conectar a las señales de ActivityManager para escuchar aperturas de recursos
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.kde.ActivityManager"),
        QStringLiteral("/Resources"),
        QStringLiteral("org.kde.ActivityManager.Resources"),
        QStringLiteral("ResourceOpened"),
        this, SLOT(onResourceOpened(QString, QString, QString))
    );
}

//! Maneja el evento de apertura de recurso (lanzamiento de aplicación)
/*!
 * Slot conectado a la señal ResourceOpened de KActivities::ActivityManager.
 * Se dispara cada vez que el usuario abre/lanza una aplicación.
 * 
 * **Proceso:**
 * 1. Extrae el appId del recurso (elimina ruta de carpeta si existe)
 * 2. Registra la aplicación en la lista de activas con timestamp actual
 * 3. Incrementa el contador de lanzamientos
 * 4. Recalcula el desgaste usando la fórmula ponderada
 * 5. Persiste los cambios a KConfig
 * 
 * @param activity ID de la actividad (no usado actualmente)
 * @param agent Servicio que reportó el evento (ej: "org.kde.plasma.desktop")
 * @param resource Ruta o URI del recurso abierto (ej: "/usr/bin/firefox")
 */
void UsageTracker::onResourceOpened(const QString &activity, const QString &agent, const QString &resource)
{
    Q_UNUSED(activity);
    qDebug() << "Resource opened:" << resource << "by agent:" << agent;
    
    // Extraer appId limpio (último componente de la ruta)
    QString appId = resource;
    if (resource.contains(QLatin1Char('/'))) {
        appId = resource.section(QLatin1Char('/'), -1);
    }

    // Registrar que la aplicación está activa (para tracking de tiempo)
    if (!m_activeApplications.contains(appId)) {
        m_activeApplications[appId] = QDateTime::currentDateTime();
    }

    // Inicializar o actualizar registro de la aplicación
    if (!m_appWearData.contains(appId)) {
        m_appWearData[appId] = AppWearInfo();
    }
    
    // Incrementar estadísticas
    m_appWearData[appId].launches++;
    m_appWearData[appId].lastOpenTime = QDateTime::currentDateTime();
    
    // Recalcular desgaste con nueva fórmula ponderada
    updateWearLevel(appId);
    saveConfig();
}

//! Recalcula y actualiza el nivel de desgaste de una aplicación
/*!
 * **Fórmula de desgaste ponderada (realista y elegante):**
 * ```
 * desgaste = (launches × LAUNCH_WEAR_FACTOR) + (activeMinutes × TIME_WEAR_FACTOR)
 *          = (launches × 1.0) + (activeMinutes × 0.01)
 * ```
 * 
 * **Ejemplos:**
 * - VS Code abierto 50 veces, 0 min activo: desgaste = 50
 * - Firefox abierto 2 veces, 2880 min activos (2 días): desgaste = 2 + 28.8 = 30.8
 * - Editor de texto 100 lanzamientos, 1 min promedio: desgaste ≈ 101
 * 
 * **Normalización:** Se capped a MAX_WEAR_LEVEL (100) para evitar sobrepasarse.
 * 
 * Emite señal wearLevelChanged() para notificar al Plasmoid.
 * 
 * @param appId Identificador de la aplicación a actualizar
 */
void UsageTracker::updateWearLevel(const QString &appId)
{
    if (!m_appWearData.contains(appId)) {
        return;
    }

    AppWearInfo &info = m_appWearData[appId];
    
    // Calcular componentes de desgaste (ponderado)
    float wearFromLaunches = info.launches * LAUNCH_WEAR_FACTOR;
    float wearFromTime = (info.activeTimeSeconds / 60.0f) * TIME_WEAR_FACTOR;  // Convertir segundos a minutos
    
    int totalWear = static_cast<int>(wearFromLaunches + wearFromTime);
    info.wearLevel = qMin(MAX_WEAR_LEVEL, totalWear);
    
    qDebug() << "App:" << appId 
             << "| Launches:" << info.launches 
             << "| Active mins:" << (info.activeTimeSeconds / 60.0f)
             << "| Total wear:" << info.wearLevel;
    
    Q_EMIT wearLevelChanged(appId, info.wearLevel);
}

//! Verifica aplicaciones activas y acumula tiempo de sesión
void UsageTracker::checkActiveApplications()
{
    QDateTime now = QDateTime::currentDateTime();
    QStringList appsToRemove;

    for (auto it = m_activeApplications.begin(); it != m_activeApplications.end(); ++it) {
        QString appId = it.key();
        QDateTime openTime = it.value();
        
        qint64 secondsElapsed = openTime.secsTo(now);
        
        // Timeout: si > 5 min sin actividad, se asume que fue cerrada
        if (secondsElapsed > 300) {
            appsToRemove.append(appId);
        } else {
            // Acumular +30 segundos (el timer ejecuta cada 30 seg)
            if (m_appWearData.contains(appId)) {
                m_appWearData[appId].activeTimeSeconds += 30;
                updateWearLevel(appId);
            }
        }
    }

    // Remover aplicaciones inactivas
    for (const QString &appId : appsToRemove) {
        m_activeApplications.remove(appId);
        qDebug() << "App cerrada (inactiva):" << appId;
    }

    if (!appsToRemove.isEmpty()) {
        saveConfig();
    }
}

void UsageTracker::loadConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("iconwearrc")), QStringLiteral("Applications"));
    const QStringList keys = config.keyList();
    
    for (const QString &appId : keys) {
        AppWearInfo info;
        KConfigGroup appGroup = config.group(appId);
        info.wearLevel = appGroup.readEntry(QStringLiteral("wearLevel"), 0);
        info.launches = appGroup.readEntry(QStringLiteral("launches"), 0);
        info.activeTimeSeconds = appGroup.readEntry(QStringLiteral("activeTimeSeconds"), 0LL);
        info.reconstructions = appGroup.readEntry(QStringLiteral("reconstructions"), 0);
        m_appWearData[appId] = info;
    }
    
    qDebug() << "Configuración cargada para" << keys.size() << "aplicaciones";
}

void UsageTracker::saveConfig()
{
    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("iconwearrc")), QStringLiteral("Applications"));
    
    // Limpiar configuración antigua
    config.deleteGroup();
    
    // Guardar datos actuales
    for (auto it = m_appWearData.begin(); it != m_appWearData.end(); ++it) {
        const QString &appId = it.key();
        const AppWearInfo &info = it.value();
        
        KConfigGroup appGroup = config.group(appId);
        appGroup.writeEntry(QStringLiteral("wearLevel"), info.wearLevel);
        appGroup.writeEntry(QStringLiteral("launches"), info.launches);
        appGroup.writeEntry(QStringLiteral("activeTimeSeconds"), info.activeTimeSeconds);
        appGroup.writeEntry(QStringLiteral("reconstructions"), info.reconstructions);
    }
    
    config.sync();
}

int UsageTracker::getWearLevel(const QString &appId)
{
    if (m_appWearData.contains(appId)) {
        return m_appWearData[appId].wearLevel;
    }
    return 0;
}

QString UsageTracker::getMetrics(const QString &appId)
{
    QJsonObject metrics;
    
    if (m_appWearData.contains(appId)) {
        const AppWearInfo &info = m_appWearData[appId];
        metrics[QStringLiteral("appId")] = appId;
        metrics[QStringLiteral("wearLevel")] = info.wearLevel;
        metrics[QStringLiteral("launches")] = info.launches;
        metrics[QStringLiteral("activeMinutes")] = static_cast<int>(info.activeTimeSeconds / 60);
        metrics[QStringLiteral("reconstructions")] = info.reconstructions;
        
        if (info.lastOpenTime.isValid()) {
            metrics[QStringLiteral("lastOpenTime")] = info.lastOpenTime.toString(Qt::ISODate);
        }
        if (info.lastResetTime.isValid()) {
            metrics[QStringLiteral("lastResetTime")] = info.lastResetTime.toString(Qt::ISODate);
        }
    } else {
        metrics[QStringLiteral("appId")] = appId;
        metrics[QStringLiteral("wearLevel")] = 0;
        metrics[QStringLiteral("launches")] = 0;
        metrics[QStringLiteral("activeMinutes")] = 0;
        metrics[QStringLiteral("reconstructions")] = 0;
    }
    
    QJsonDocument doc(metrics);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

void UsageTracker::resetWearLevel(const QString &appId)
{
    if (!m_appWearData.contains(appId)) {
        return;
    }
    
    AppWearInfo &info = m_appWearData[appId];
    
    // Resetear desgaste pero mantener historial
    info.wearLevel = 0;
    info.reconstructions++;
    info.lastResetTime = QDateTime::currentDateTime();
    
    qDebug() << "App" << appId << "reseteada. Reconstrucciones:" << info.reconstructions;
    
    // Guardar configuración
    saveConfig();
    
    // Emitir señales
    Q_EMIT wearLevelChanged(appId, 0);
    Q_EMIT wearLevelReset(appId);
}

int UsageTracker::getReconstructions(const QString &appId)
{
    if (m_appWearData.contains(appId)) {
        return m_appWearData[appId].reconstructions;
    }
    return 0;
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

        if (m_appWearData[appId].wearLevel != wearLevel) {
            m_appWearData[appId].wearLevel = wearLevel;
            Q_EMIT wearLevelChanged(appId, wearLevel);
        }
    }
}
