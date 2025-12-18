/**
 * @file main.qml
 * @brief Interfaz principal del widget Plasmoid IconWear
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * 
 * Componente QML que implementa la UI del widget de Plasma.
 * Maneja:
 * - Visualización del icono con efecto de desgaste (shader)
 * - Menú contextual para opciones de reset y métricas
 * - Tooltip con estadísticas en tiempo real
 * - Slider interactivo para testing
 * 
 * Se comunica con el daemon UsageTracker via DBus para obtener/actualizar datos.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

/**
 * @class Item (root element)
 * @brief Contenedor principal del widget
 * 
 * Propiedades:
 * - appId: Identificador de aplicación siendo monitoreada
 * - wearLevel: Nivel de desgaste (0-100) que controla visual del shader
 * - launches: Número de lanzamientos de la app
 * - activeMinutes: Minutos totales que la app ha estado activa
 * - reconstructions: Contador de veces que fue "restaurada"
 * - showMetrics: Flag para mostrar/ocultar el tooltip
 */
Item {
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    property string appId: "org.kde.dolphin.desktop"    ///< App siendo monitoreada
    property real wearLevel: 0.0                         ///< Desgaste 0-100
    property int launches: 0                             ///< Número de lanzamientos
    property int reconstructions: 0                      ///< Contador de resets
    property int activeMinutes: 0                        ///< Minutos activos acumulados
    property bool showMetrics: false                     ///< Mostrar tooltip con stats

    // DBus connection to the daemon
    PlasmaCore.DataSource {
        id: dbusSource
        engine: "executable"
        connectedSources: []
        onNewData: {
            // Handle DBus responses if needed
        }
    }

    // For simplicity in this MVP, we'll use a Timer to poll or just hardcode for now
    // In a real app, we'd use a proper DBus interface in QML
    Timer {
        interval: 5000
        running: true
        repeat: true
        onTriggered: {
            // Mocking DBus call for now
            // wearLevel = ...
        }
    }

    /**
     * @brief Menú contextual (clic derecho)
     * 
     * Opciones principales:
     * - Ver Métricas: Toggle del tooltip informativo
     * - Restaurar Icono: Reset con animación de spark + contador
     */
    PlasmaComponents.Menu {
        id: contextMenu
        
        PlasmaComponents.MenuItem {
            text: "Ver Métricas"
            icon.name: "dialog-information"
            onClicked: showMetrics = !showMetrics
        }
        
        PlasmaComponents.MenuSeparator {}
        
        PlasmaComponents.MenuItem {
            text: "Restaurar Icono"
            icon.name: "edit-undo"
            onClicked: resetWearAnimation()
        }
    }

    /**
     * @brief Anima reset visual y llama al daemon
     * 
     * Proceso:
     * 1. Ejecuta animación de spark/glitch en shader
     * 2. Pone wearLevel a 0
     * 3. Incrementa contador de reconstrucciones
     * 4. Llamaría a daemon.resetWearLevel(appId) en versión final
     * 
     * @see WearShader.playResetAnimation()
     * @see UsageTracker.resetWearLevel()
     */
    function resetWearAnimation() {
        // Disparar animación de spark inverso en el shader
        wearShaderComponent.playResetAnimation()
        
        // Resetear visualmente
        wearLevel = 0
        reconstructions += 1
        
        console.log("Icono restaurado. Restauraciones:", reconstructions)
        // TODO: Llamar a daemon.resetWearLevel(appId) via DBus
    }

    /**
     * @brief Layout principal con icono, tooltip y controles
     */
    ColumnLayout {
        anchors.fill: parent
        
        /**
         * @brief Contenedor del icono con efecto visual
         * 
         * Estructura:
         * - IconItem: Icono base KDE (oculto)
         * - WearShader: Efecto GLSL de desgaste en tiempo real
         * - MouseArea: Maneja clic derecho y hover
         */
        Item {
            id: iconContainer
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.alignment: Qt.AlignCenter

            // Icono base de KDE (renderizado por el shader)
            PlasmaCore.IconItem {
                id: iconItem
                anchors.fill: parent
                source: "system-file-manager"
                visible: false // El shader lo renderiza, no este
            }

            /**
             * @brief Aplica efecto visual de desgaste (shader GLSL)
             * 
             * Propiedades:
             * - source: Icono base a procesar
             * - wearLevel: 0.0-1.0 (normalizado)
             * - resetProgress: Animación de reset en progreso
             * 
             * @see WearShader.qml para fórmulas de shader
             */
            WearShader {
                id: wearShaderComponent
                anchors.fill: parent
                source: iconItem
                wearLevel: parent.parent.parent.wearLevel / 100.0
            }

            /**
             * @brief Detector de interacción del usuario
             * 
             * - Clic derecho: Muestra menú contextual
             * - Hover: Muestra/oculta tooltip de métricas
             */
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton | Qt.LeftButton
                
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.popup()
                    }
                }
                
                onEntered: {
                    tooltipComponent.visible = true
                }
                
                onExited: {
                    tooltipComponent.visible = false
                }
            }
        }

        /**
         * @brief Tooltip con estadísticas del icono
         * 
         * Aparece al pasar el mouse y muestra:
         * - Aperturas: Número de lanzamientos
         * - Desgaste: Nivel 0-100%
         * - Activo: Minutos totales
         * - Restauraciones: Contador de resets
         * 
         * Estilo: Caja semi-transparente con borde
         */
        Rectangle {
            id: tooltipComponent
            visible: false
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 8
            color: PlasmaCore.ColorScope.backgroundColor
            border.color: PlasmaCore.ColorScope.textColor
            border.width: 1
            radius: 4
            width: metricsText.width + 12
            height: metricsText.height + 12
            
            Text {
                id: metricsText
                anchors.centerIn: parent
                text: "Uso: " + launches + " aperturas\n" +
                      "Desgaste: " + Math.round(wearLevel) + "%\n" +
                      "Activo: " + activeMinutes + " min\n" +
                      "Restauraciones: " + reconstructions
                font.pixelSize: 10
                color: PlasmaCore.ColorScope.textColor
            }
        }

        /// Label mostrando desgaste actual
        PlasmaComponents.Label {
            text: "Wear: " + Math.round(wearLevel) + "%"
            Layout.alignment: Qt.AlignCenter
        }
        
        /// Slider para testing manual del desgaste
        PlasmaComponents.Slider {
            from: 0
            to: 100
            value: wearLevel
            onMoved: wearLevel = value
            Layout.fillWidth: true
        }
    }
}
