/**
 * @file main.cpp
 * @brief Punto de entrada del daemon IconWear
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * 
 * Demonio de sistema (systemd service) que:
 * 1. Inicializa la aplicación Qt
 * 2. Registra el servicio DBus "org.kde.iconwear"
 * 3. Expone la interfaz UsageTracker en /Tracker
 * 4. Se mantiene escuchando eventos de actividad del sistema
 * 
 * **Instalación:**
 * ```bash
 * mkdir build && cd build
 * cmake ..
 * make
 * sudo make install
 * ```
 * 
 * **Uso:**
 * El daemon se inicia automáticamente al iniciar sesión en KDE Plasma.
 * Se comunica con el widget plasmoid via DBus.
 * 
 * **Debugging:**
 * ```bash
 * # Ejecutar manualmente para ver logs
 * ./iconwear-daemon
 * 
 * # Inspeccionar servicio DBus
 * qdbus org.kde.iconwear /Tracker
 * 
 * # Ver configuración guardada
 * cat ~/.config/iconwearrc
 * ```
 */

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>
#include "usagetracker.h"

//! Función principal - Inicializa y ejecuta el daemon
/*!
 * **Proceso de inicialización:**
 * 
 * 1. Crea QCoreApplication (event loop)
 * 2. Configura nombre de aplicación y organización
 * 3. Instancia UsageTracker:
 *    - Carga configuración guardada
 *    - Conecta a señales de ActivityManager
 *    - Inicia timer de monitoring
 * 4. Registra servicio DBus "org.kde.iconwear"
 * 5. Expone objeto /Tracker con interfaz pública
 * 6. Entra en event loop (espera eventos)
 * 
 * **Interfaces expuestas:**
 * ```
 * Service: org.kde.iconwear
 * Path: /Tracker
 * Interface: org.kde.iconwear.Tracker
 * 
 * Methods:
 *   int getWearLevel(QString appId)
 *   QString getMetrics(QString appId)
 *   void resetWearLevel(QString appId)
 *   int getReconstructions(QString appId)
 * 
 * Signals:
 *   wearLevelChanged(QString appId, int newLevel)
 *   wearLevelReset(QString appId)
 * ```
 * 
 * @return 0 si todo OK, 1 si hay error al registrar DBus
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("iconwear-daemon"));
    app.setOrganizationDomain(QStringLiteral("org.kde"));

    // Crear instancia del rastreador (inicializa todo)
    UsageTracker tracker;

    // Registrar servicio DBus
    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.iconwear"))) {
        qCritical() << "Failed to register DBus service org.kde.iconwear";
        return 1;
    }

    // Exponer objeto en la ruta /Tracker con todas sus slots y señales
    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Tracker"), &tracker, 
                                                       QDBusConnection::ExportAllSlots | 
                                                       QDBusConnection::ExportAllSignals)) {
        qCritical() << "Failed to register DBus object /Tracker";
        return 1;
    }

    qInfo() << "✓ IconWear Daemon started successfully";
    qInfo() << "  Service: org.kde.iconwear";
    qInfo() << "  Object: /Tracker";

    // Event loop - espera eventos DBus y ActivityManager
    return app.exec();
}
