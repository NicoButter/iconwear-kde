/**
 * @file usagetracker.h
 * @brief Rastreador de uso de aplicaciones para IconWear KDE
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * @date 2025
 * @license MIT
 * 
 * Este módulo implementa el core de seguimiento de uso de aplicaciones.
 * Rastrea lanzamientos, tiempo activo y calcula un nivel de desgaste
 * ponderado que representa visualmente el uso relativo de cada app.
 */

#ifndef USAGETRACKER_H
#define USAGETRACKER_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QTimer>
#include <KActivities/Stats/ResultSet>

/**
 * @struct AppWearInfo
 * @brief Información de desgaste para una aplicación individual
 * 
 * Estructura que almacena todas las métricas relacionadas con el desgaste
 * de una aplicación. El desgaste total se calcula como:
 * 
 * @code
 * desgaste = (launches * LAUNCH_WEAR_FACTOR) + (activeMinutes * TIME_WEAR_FACTOR)
 * @endcode
 * 
 * @see UsageTracker::updateWearLevel()
 */
struct AppWearInfo {
    int wearLevel = 0;              ///< Nivel de desgaste normalizado (0-100)
    int launches = 0;               ///< Número total de lanzamientos de la app
    qint64 activeTimeSeconds = 0;   ///< Tiempo total activo en segundos
    int reconstructions = 0;         ///< Contador de veces que fue reseteada
    QDateTime lastOpenTime;          ///< Timestamp de la última apertura
    QDateTime lastResetTime;         ///< Timestamp del último reset
};

/**
 * @class UsageTracker
 * @brief Servicio DBus que rastrea y gestiona el desgaste de aplicaciones
 * 
 * UsageTracker es un servicio de demonio que escucha eventos de actividad
 * del sistema KDE (a través de KActivitiesStats y ActivityManager) y mantiene
 * un registro ponderado del "desgaste" visual de cada aplicación.
 * 
 * **Características principales:**
 * - Monitoreo en tiempo real de lanzamientos de aplicaciones
 * - Seguimiento de tiempo activo de cada aplicación
 * - Cálculo de desgaste ponderado (lanzamientos + tiempo activo)
 * - Persistencia de datos en KConfig (~/.config/iconwearrc)
 * - Interfaz DBus para consultas remotas desde el Plasmoid
 * - Reset con contador de "reconstrucciones"
 * 
 * **Señales DBus:**
 * - `wearLevelChanged(appId, newLevel)` - Emitido cuando cambia el desgaste
 * - `wearLevelReset(appId)` - Emitido cuando se resetea una app
 * 
 * @note El cálculo de desgaste es **ponderado** para ser más realista:
 * - +1 punto por cada lanzamiento
 * - +0.01 puntos por cada minuto activo
 * - Máximo 100 puntos (normalizado)
 * 
 * @see AppWearInfo
 */
class UsageTracker : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.iconwear.Tracker")

public:
    /**
     * @brief Constructor del rastreador de uso
     * @param parent Objeto padre de Qt (por defecto nullptr)
     * 
     * Inicializa el servicio cargando datos guardados, configurando
     * conexiones DBus y activando el timer de monitoreo de aplicaciones.
     */
    explicit UsageTracker(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief Obtiene el nivel de desgaste de una aplicación
     * @param appId Identificador de la aplicación (ej: "org.kde.dolphin")
     * @return Nivel de desgaste (0-100), o 0 si no existe registro
     * 
     * Slot DBus que retorna el nivel de desgaste normalizado para una aplicación.
     * Se puede llamar remotamente desde el Plasmoid.
     * 
     * @see getMetrics()
     */
    int getWearLevel(const QString &appId);
    
    /**
     * @brief Obtiene métricas completas de una aplicación en formato JSON
     * @param appId Identificador de la aplicación
     * @return String JSON con las siguientes propiedades:
     *         - appId: Identificador
     *         - wearLevel: Desgaste (0-100)
     *         - launches: Número de lanzamientos
     *         - activeMinutes: Minutos totales activo
     *         - reconstructions: Número de resets
     *         - lastOpenTime: ISO8601 timestamp
     *         - lastResetTime: ISO8601 timestamp
     * 
     * Ideal para mostrar un tooltip o panel de información detallado.
     * 
     * @see getWearLevel()
     */
    QString getMetrics(const QString &appId);
    
    /**
     * @brief Resetea el desgaste de una aplicación (reconstrucción)
     * @param appId Identificador de la aplicación
     * 
     * Pone el desgaste a 0 pero mantiene un histórico de reconstrucciones.
     * Emite señal wearLevelReset() para activar animación en el Plasmoid.
     * 
     * Persiste automáticamente los cambios en KConfig.
     * 
     * @see getReconstructions()
     */
    void resetWearLevel(const QString &appId);
    
    /**
     * @brief Obtiene el número de reconstrucciones de una aplicación
     * @param appId Identificador de la aplicación
     * @return Contador de resets, o 0 si nunca fue reseteada
     * 
     * Útil para mostrar un badge o contador visual de reconstrucciones.
     */
    int getReconstructions(const QString &appId);

private Q_SLOTS:
    /**
     * @brief Maneja el evento de apertura de recurso (aplicación)
     * @param activity ID de la actividad actual
     * @param agent Agente que reportó el evento (ej: "org.kde.plasma.desktop")
     * @param resource Recurso abierto (ruta o URI de la aplicación)
     * 
     * Slot privado conectado a la señal ResourceOpened de ActivityManager.
     * Incrementa el contador de lanzamientos y actualiza el desgaste.
     * 
     * @note El parámetro `activity` no se usa actualmente pero se mantiene
     *       por compatibilidad con la interfaz de ActivityManager.
     */
    void onResourceOpened(const QString &activity, const QString &agent, const QString &resource);
    
    /**
     * @brief Verifica aplicaciones activas y actualiza tiempo de sesión
     * 
     * Slot privado conectado a un timer que se ejecuta cada 30 segundos.
     * Detecta aplicaciones que se cerraron (sin actividad en 5 minutos)
     * y acumula tiempo activo para las que siguen en ejecución.
     * 
     * Automáticamente persiste cambios cuando detecta inactividad.
     */
    void checkActiveApplications();

Q_SIGNALS:
    /**
     * @brief Señal emitida cuando cambia el nivel de desgaste de una app
     * @param appId Identificador de la aplicación
     * @param newLevel Nuevo nivel de desgaste (0-100)
     * 
     * Usada por el Plasmoid para actualizar visualización en tiempo real.
     * Se emite cada vez que se calcula desgaste después de un evento.
     */
    void wearLevelChanged(const QString &appId, int newLevel);
    
    /**
     * @brief Señal emitida cuando se resetea el desgaste de una app
     * @param appId Identificador de la aplicación
     * 
     * Usada por el Plasmoid para activar animación de "spark/glitch inverso"
     * que visualiza la reconstrucción del icono.
     * 
     * @see WearShader::playResetAnimation()
     */
    void wearLevelReset(const QString &appId);

private:
    /**
     * @brief Recalcula el nivel de desgaste de una aplicación
     * @param appId Identificador de la aplicación
     * 
     * Calcula el desgaste usando la fórmula ponderada:
     * `desgaste = (launches * LAUNCH_WEAR_FACTOR) + (activeMinutes * TIME_WEAR_FACTOR)`
     * 
     * Normaliza a 0-100 y emite señal wearLevelChanged().
     * No persiste automáticamente (ver saveConfig()).
     * 
     * @see LAUNCH_WEAR_FACTOR, TIME_WEAR_FACTOR, MAX_WEAR_LEVEL
     */
    void updateWearLevel(const QString &appId);
    
    /**
     * @brief Actualiza estadísticas desde KActivitiesStats
     * 
     * Lee datos históricos de KActivities para inicializar o refrescar
     * métricas. Se ejecuta al iniciar el daemon.
     * 
     * @note Actualmente es un placeholder para integración futura.
     */
    void updateStats();
    
    /**
     * @brief Carga configuración guardada desde KConfig
     * 
     * Lee ~/.config/iconwearrc y restaura el estado de todas las
     * aplicaciones (desgaste, lanzamientos, tiempo activo, etc.).
     * Se ejecuta en el constructor.
     * 
     * @see saveConfig()
     */
    void loadConfig();
    
    /**
     * @brief Persiste el estado actual a KConfig
     * 
     * Escribe todos los datos en ~/.config/iconwearrc/Applications
     * para que sobrevivan reinicio de sesión.
     * Se llama automáticamente después de cambios.
     * 
     * @see loadConfig()
     */
    void saveConfig();
    
    // ============= Configuración de Factores de Desgaste =============
    
    /// Desgaste agregado por cada lanzamiento de aplicación
    static constexpr float LAUNCH_WEAR_FACTOR = 1.0f;
    
    /// Desgaste agregado por cada minuto de tiempo activo (más lento que lanzamientos)
    static constexpr float TIME_WEAR_FACTOR = 0.01f;
    
    /// Nivel máximo de desgaste (normalización superior)
    static constexpr int MAX_WEAR_LEVEL = 100;
    
    // ============= Miembros Privados =============
    
    /// Mapa principal: appId -> AppWearInfo con todas las métricas
    QMap<QString, AppWearInfo> m_appWearData;
    
    /// Mapa de aplicaciones actualmente en ejecución y su tiempo de apertura
    QMap<QString, QDateTime> m_activeApplications;
    
    /// Timer que ejecuta checkActiveApplications() cada 30 segundos
    QTimer *m_activityCheckTimer;
};

#endif // USAGETRACKER_H
