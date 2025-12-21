/**
 * @file WearShader.qml
 * @brief Efecto visual de desgaste a iconos (Qt6 compatible)
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * 
 * Implementa un efecto visual que transforma el icono base aplicando:
 * - Desaturación progresiva
 * - Grietas/roturas que aparecen con el uso
 * - Animación de "spark" inverso en reset (flash blanco)
 */

import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

/**
 * @class Item
 * @brief Contenedor del efecto de desgaste visual
 */
Item {
    id: root
    property var source
    property real wearLevel: 0.0
    property real resetProgress: 0.0

    SequentialAnimation {
        id: resetAnimation
        
        PropertyAnimation {
            target: root
            property: "resetProgress"
            from: 0.0
            to: 1.0
            duration: 300
            easing.type: Easing.OutQuad
        }
        
        PropertyAnimation {
            target: root
            property: "resetProgress"
            from: 1.0
            to: 0.0
            duration: 150
            easing.type: Easing.InQuad
        }
    }

    function playResetAnimation() {
        resetAnimation.start()
    }

    // Renderizar el icono fuente
    ShaderEffectSource {
        id: effectSource
        sourceItem: root.source
        anchors.fill: parent
        visible: false
        hideSource: true
    }

    // Efecto de desaturación basado en wearLevel
    Desaturate {
        id: desaturateEffect
        anchors.fill: parent
        source: effectSource
        desaturation: root.wearLevel * 0.7
        visible: true
    }

    // Efecto de oscurecimiento
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: root.wearLevel * 0.25
    }

    // === GRIETAS Y ROTURAS ===
    Canvas {
        id: cracksCanvas
        anchors.fill: parent
        visible: root.wearLevel > 0.1
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            if (root.wearLevel < 0.1) return;
            
            ctx.strokeStyle = Qt.rgba(0, 0, 0, 0.6 * root.wearLevel);
            ctx.lineWidth = 1 + root.wearLevel;
            
            // Semilla basada en tamaño para consistencia
            var seed = width * height;
            
            // Número de grietas basado en wearLevel
            var numCracks = Math.floor(root.wearLevel * 8);
            
            for (var i = 0; i < numCracks; i++) {
                drawCrack(ctx, i, seed);
            }
        }
        
        function drawCrack(ctx, index, seed) {
            // Posición inicial pseudo-aleatoria pero consistente
            var startX = ((seed * (index + 1) * 13) % 100) / 100 * width;
            var startY = ((seed * (index + 1) * 17) % 100) / 100 * height;
            
            ctx.beginPath();
            ctx.moveTo(startX, startY);
            
            var segments = 3 + Math.floor(root.wearLevel * 4);
            var x = startX;
            var y = startY;
            
            for (var j = 0; j < segments; j++) {
                // Dirección pseudo-aleatoria
                var angle = ((seed * (index + 1) * (j + 1) * 23) % 360) * Math.PI / 180;
                var length = 5 + root.wearLevel * 15;
                
                x += Math.cos(angle) * length;
                y += Math.sin(angle) * length;
                
                // Mantener dentro de límites
                x = Math.max(0, Math.min(width, x));
                y = Math.max(0, Math.min(height, y));
                
                ctx.lineTo(x, y);
            }
            
            ctx.stroke();
        }
        
        // Redibujar cuando cambia wearLevel
        Connections {
            target: root
            function onWearLevelChanged() {
                cracksCanvas.requestPaint();
            }
        }
    }
    
    // Puntos de impacto / astillas
    Repeater {
        model: Math.floor(root.wearLevel * 6)
        
        Rectangle {
            id: chip
            width: 2 + root.wearLevel * 4
            height: width
            radius: width / 2
            color: Qt.rgba(0.3, 0.3, 0.3, 0.5 * root.wearLevel)
            
            // Posición pseudo-aleatoria basada en índice
            x: ((index * 37 + 13) % 100) / 100 * (parent.width - width)
            y: ((index * 53 + 7) % 100) / 100 * (parent.height - height)
            
            visible: root.wearLevel > 0.2
        }
    }
    
    // Arañazos diagonales
    Repeater {
        model: Math.floor(root.wearLevel * 4)
        
        Rectangle {
            width: 1
            height: 8 + root.wearLevel * 12
            color: Qt.rgba(0.8, 0.8, 0.8, 0.4 * root.wearLevel)
            rotation: 30 + (index * 25)
            
            x: ((index * 41 + 19) % 100) / 100 * parent.width
            y: ((index * 61 + 11) % 100) / 100 * parent.height
            
            visible: root.wearLevel > 0.15
        }
    }

    // Efecto de flash blanco para animación de reset
    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: root.resetProgress * 0.8
        visible: root.resetProgress > 0
    }
}
