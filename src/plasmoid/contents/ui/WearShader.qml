import QtQuick 2.15
import QtQuick.Controls 2.15

ShaderEffect {
    property variant source
    property real wearLevel: 0.0 // 0.0 to 1.0

    fragmentShader: "
        varying highp vec2 qt_TexCoord0;
        uniform lowp float qt_Opacity;
        uniform sampler2D source;
        uniform highp float wearLevel;

        highp float rand(vec2 co) {
            return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
        }

        void main() {
            lowp vec4 tex = texture2D(source, qt_TexCoord0);
            
            // Simple scratch effect based on noise
            highp float noise = rand(qt_TexCoord0 * 100.0);
            highp float scratch = step(0.98 - (wearLevel * 0.1), noise);
            
            // Fading effect
            lowp vec4 wornColor = tex;
            wornColor.rgb *= (1.0 - (wearLevel * 0.3)); // Darken/Fade
            
            // Apply scratches (white/grey marks)
            wornColor.rgb = mix(wornColor.rgb, vec3(0.8), scratch * wearLevel);
            
            gl_FragColor = wornColor * qt_Opacity;
        }
    "
}
