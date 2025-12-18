/**
 * @file WearShader.qml
 * @brief Shader GLSL que aplica efecto visual de desgaste a iconos
 * @author Nicolas Butterfield <nicobutter@gmail.com>
 * 
 * Implementa un ShaderEffect que transforma el icono base aplicando:
 * - Ruido procedural (scratches/arañazos)
 * - Desaturación progresiva
 * - Animación de "spark" inverso en reset (reconstrucción)
 * 
 * **Filosofía visual:**
 * El desgaste se visualiza como:
 * 1. Arañazos aleatorios crecientes
 * 2. Pérdida gradual de saturación de color
 * 3. Brillo recuperado durante reset (animación esperanzadora)
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * @class ShaderEffect
 * @brief Efecto GLSL 2D para desgaste visual
 * 
 * Propiedades principales:
 * - source: IconItem base a procesar
 * - wearLevel: 0.0-1.0 intensidad de desgaste
 * - resetProgress: 0.0-1.0 animación de reset
 */
ShaderEffect {
    property variant source                    ///< Icono KDE a procesar
    property real wearLevel: 0.0               ///< Desgaste 0.0 (nuevo) a 1.0 (destruido)
    property real resetProgress: 0.0           ///< Progreso de animación reset 0.0-1.0

    /**
     * @brief Animación de reconstrucción visual
     * 
     * Secuencia:
     * 1. Flash blanco brillante (300ms OUT_QUAD): Destella de energía
     * 2. Desvanecimiento (150ms IN_QUAD): Vuelve a normal
     * 
     * Produce efecto "spark" o "glitch inverso" visual muy satisfactorio
     */
    SequentialAnimation {
        id: resetAnimation
        
        PropertyAnimation {
            target: parent
            property: "resetProgress"
            from: 0.0
            to: 1.0
            duration: 300
            easing.type: Easing.OutQuad
        }
        
        PropertyAnimation {
            target: parent
            property: "resetProgress"
            from: 1.0
            to: 0.0
            duration: 150
            easing.type: Easing.InQuad
        }
    }

    /**
     * @brief Dispara la animación de reconstrucción visual
     * 
     * Se llama desde main.qml cuando el usuario hace clic "Restaurar Icono".
     * Ejecuta resetAnimation que modula resetProgress de 0→1→0.
     * 
     * @see resetAnimation SequentialAnimation
     */
    function playResetAnimation() {
        resetAnimation.start()
    }

    /**
     * @brief Fragment Shader GLSL - Efecto de desgaste en tiempo real
     * 
     * **Uniforms (entradas):**
     * - wearLevel: 0.0-1.0 intensidad de desgaste
     * - resetProgress: 0.0-1.0 animación de reset
     * - qt_TexCoord0: Coordenadas UV del texel
     * - source: Sampler del icono base
     * 
     * **Algoritmo:**
     * 
     * 1. **Ruido procedural (scratches):**
     *    - rand(): Función 2D pseudo-aleatoria basada en sin(dot())
     *    - Genera patrón de ruido determinístico por posición
     *    - Aumenta con wearLevel para crear arañazos crecientes
     * 
     * 2. **Desaturación (fade):**
     *    - Multiplica RGB por (1 - wearLevel × 0.3)
     *    - Efecto visual: icono se oscurece/desvanece con uso
     * 
     * 3. **Arañazos blancos:**
     *    - Mix() combina color normal con gris (0.8)
     *    - Proporcional a wearLevel: más desgaste = más arañazos
     * 
     * 4. **Animación reset (spark):**
     *    - Si resetProgress > 0: overlay blanco brillante
     *    - Modula sparks usando resetProgress
     *    - Efecto: destello de "reconstrucción"
     * 
     * **Fórmula final:**
     * ```glsl
     * color = mix(desgastado, spark, sparkMask × resetProgress)
     * gl_FragColor = color × alphaGlobal
     * ```
     */
    fragmentShader: "
        varying highp vec2 qt_TexCoord0;
        uniform lowp float qt_Opacity;
        uniform sampler2D source;
        uniform highp float wearLevel;
        uniform highp float resetProgress;

        /// Generador de ruido pseudoaleatorio 2D
        /// @param co Coordenadas 2D (típicamente UV × escala)
        /// @return Float 0.0-1.0 aparentemente aleatorio pero determinístico por posición
        highp float rand(vec2 co) {
            return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
        }

        void main() {
            // Muestrear texel original
            lowp vec4 tex = texture2D(source, qt_TexCoord0);
            
            // === PASO 1: Efecto de arañazos (scratches) ===
            highp float noise = rand(qt_TexCoord0 * 100.0);
            highp float scratch = step(0.98 - (wearLevel * 0.1), noise);
            
            // === PASO 2: Desaturación/oscurecimiento ===
            lowp vec4 wornColor = tex;
            wornColor.rgb *= (1.0 - (wearLevel * 0.3));
            
            // === PASO 3: Aplicar marcas de arañazos en blanco/gris ===
            wornColor.rgb = mix(wornColor.rgb, vec3(0.8), scratch * wearLevel);
            
            // === PASO 4: Animación de reset (spark/glitch inverso) ===
            if (resetProgress > 0.0) {
                highp float sparkNoise = rand(qt_TexCoord0 * 200.0 + resetProgress);
                highp float sparkMask = step(0.6 - resetProgress * 0.5, sparkNoise);
                // Overlay blanco brillante modulado por animación
                wornColor.rgb = mix(wornColor.rgb, vec3(1.0), sparkMask * resetProgress * 0.8);
            }
            
            gl_FragColor = wornColor * qt_Opacity;
        }
    "
}
