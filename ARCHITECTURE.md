# IconWear KDE - Arquitectura del Sistema

**Autor:** Nicolas Butterfield  
**Email:** nicobutter@gmail.com  
**Fecha:** Diciembre 2025  
**Licencia:** MIT

---

## 📋 Resumen Ejecutivo

IconWear es una aplicación KDE Plasma que añade una dimensión visual al escritorio: **los iconos envejecen según el uso**. Mientras más usas una aplicación, su icono se ve progresivamente desgastado (rayones, desaturación), hasta que puedes "restaurarlo" resetando su apariencia.

**Concepto:** Tu escritorio cuenta la historia de tu flujo de trabajo.

---

## 🏗️ Arquitectura General

```
┌─────────────────────────────────────────────────────────────┐
│                    KDE Plasma Desktop                        │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │         Plasmoid Widget (UI Frontend)                │   │
│  │                                                       │   │
│  │  ┌─────────────────┐  ┌─────────────────┐           │   │
│  │  │  main.qml       │  │  WearShader.qml │           │   │
│  │  │  (UI Logic)     │  │  (GLSL Effects) │           │   │
│  │  └────────┬────────┘  └────────┬────────┘           │   │
│  │           │                    │                     │   │
│  │           └────────┬───────────┘                     │   │
│  │                    │ DBus API Calls                  │   │
│  └────────────────────┼─────────────────────────────────┘   │
│                       │                                      │
│                       │ Session Bus                          │
│                       │ org.kde.iconwear                     │
│                       ▼                                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │    iconwear-daemon (Backend Service)                 │   │
│  │                                                       │   │
│  │  ┌───────────────────────────────────────────────┐   │   │
│  │  │          UsageTracker (Core Logic)            │   │   │
│  │  │                                               │   │   │
│  │  │  • Tracking de lanzamientos                  │   │   │
│  │  │  • Acumulación de tiempo activo              │   │   │
│  │  │  • Cálculo de desgaste ponderado             │   │   │
│  │  │  • Persistencia en KConfig                   │   │   │
│  │  │  • Interfaz DBus                             │   │   │
│  │  └───────────────────────────────────────────────┘   │   │
│  │                 ▲        │       ▲                    │   │
│  │                 │        │       │                    │   │
│  │          KConfig│        │       │Signals            │   │
│  │          ╱────────────────┘       │                   │   │
│  │         │                         │                   │   │
│  │         ▼                         ▼                   │   │
│  │  ~/.config/        org.kde.ActivityManager           │   │
│  │  iconwearrc        (System Event Bus)                │   │
│  │                                                       │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔄 Flujo de Datos

### 1. **Lanzamiento de Aplicación**

```
┌─────────────────────────┐
│ Usuario abre app        │
│ (ej: Firefox)           │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────┐
│ KDE ActivityManager                                 │
│ Emite señal ResourceOpened                         │
│ - activity: UUID de actividad                      │
│ - agent: "org.kde.plasma.desktop"                  │
│ - resource: "/usr/bin/firefox" o similar           │
└────────────┬────────────────────────────────────────┘
             │ Signal DBus
             ▼
┌──────────────────────────────────────────────────┐
│ daemon: onResourceOpened()                        │
├──────────────────────────────────────────────────┤
│ 1. Extrae appId = "firefox"                      │
│ 2. Si NO existe registro: crea AppWearInfo       │
│ 3. Si existe: incrementa launches++              │
│ 4. Registra en m_activeApplications[appId]       │
│ 5. Llama updateWearLevel(appId)                  │
│ 6. Emite wearLevelChanged(appId, newLevel)       │
│ 7. saveConfig() persiste a ~/.config/iconwearrc │
└────────────┬─────────────────────────────────────┘
             │
             ▼
┌──────────────────────────────────────────────────┐
│ Plasmoid: Recibe signal wearLevelChanged()       │
├──────────────────────────────────────────────────┤
│ 1. Actualiza propiedad wearLevel                │
│ 2. Shader recalcula efectos visuales             │
│ 3. Icono muestra más "desgaste" visual           │
└──────────────────────────────────────────────────┘
```

### 2. **Acumulación de Tiempo Activo (cada 30 segundos)**

```
┌──────────────────────────┐
│ Timer dispara cada 30seg │
│ checkActiveApplications()│
└────────────┬─────────────┘
             │
             ▼
┌───────────────────────────────────────────────┐
│ Para cada app en m_activeApplications:        │
├───────────────────────────────────────────────┤
│ A) Si inactiva > 5min:                        │
│    - Remover de activeApplications            │
│    - Indica que fue cerrada                   │
│                                               │
│ B) Si aún activa:                             │
│    - activeTimeSeconds[appId] += 30           │
│    - Llamar updateWearLevel(appId)            │
│    - Emite wearLevelChanged()                 │
└───────────────────────────────────────────────┘
```

---

## 📊 Fórmula de Desgaste

El desgaste se calcula de forma **realista y ponderada**:

```
DESGASTE = (launches × LAUNCH_WEAR_FACTOR) + (activeMinutes × TIME_WEAR_FACTOR)
         = (launches × 1.0) + (activeMinutes × 0.01)
         = Capped a MAX_WEAR_LEVEL (100)
```

### Ejemplos Concretos:

| Aplicación | Lanzamientos | Minutos Activo | Cálculo | Desgaste |
|---|---|---|---|---|
| **VS Code** | 50 | 0 | (50×1.0) + (0×0.01) | **50** |
| **Firefox** | 2 | 2880 (2 días) | (2×1.0) + (2880×0.01) | **30.8** |
| **Terminal** | 100 | 100 | (100×1.0) + (100×0.01) | **101 → 100** |
| **Blender** | 10 | 5000 (3+ días) | (10×1.0) + (5000×0.01) | **60** |

**¿Por qué es mejor que solo contar lanzamientos?**

- ❌ **Problema anterior:** Abrir VS Code 50 veces = destruido, pero Firefox abierto 2 años = poco desgaste
- ✅ **Solución actual:** Pondera tanto lanzamientos como tiempo de uso
- ✅ **Realista:** Una app que usas todo el día se ve más "gastada" que una que abres muchas veces pero brevemente

---

## 🗂️ Estructura de Ficheros

```
iconwear-kde/
│
├── src/
│   ├── daemon/                         # Backend (systemd service)
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp                    # Punto de entrada, registra DBus
│   │   ├── usagetracker.h              # Header con interfaz pública
│   │   └── usagetracker.cpp            # Implementación del core
│   │
│   └── plasmoid/                       # Frontend (Plasma Widget)
│       ├── metadata.json               # Metadatos del widget
│       └── contents/ui/
│           ├── main.qml                # UI principal
│           └── WearShader.qml          # Shader GLSL de efectos
│
├── CMakeLists.txt                      # Build config general
├── README.md                           # Documentación para usuarios
├── ARCHITECTURE.md                     # Este archivo
├── DEPENDENCIES.md                     # Requisitos del sistema
├── .gitignore                          # Excepciones git
└── LICENSE                             # MIT License
```

---

## 🔐 Persistencia de Datos

### KConfig Storage (~/. config/iconwearrc)

```ini
[Applications]

[Applications/firefox]
wearLevel=45
launches=12
activeTimeSeconds=8640
reconstructions=2

[Applications/org.kde.dolphin]
wearLevel=78
launches=247
activeTimeSeconds=3600
reconstructions=0

[Applications/gedit]
wearLevel=100
launches=89
activeTimeSeconds=2400
reconstructions=1
```

**Formato:** Grupos anidados con entradas clave-valor
**Ventajas:**
- Nativo de KDE, integrado con Settings
- Auto-sincronización entre sesiones
- Compatible con herramientas KDE existentes
- Fácil debugging (archivo de texto)

---

## 🌐 Interfaz DBus

### Servicio Principal

```
Service:  org.kde.iconwear
Path:     /Tracker
Interface: org.kde.iconwear.Tracker
```

### Métodos Disponibles

```python
# Obtener desgaste actual (0-100)
int getWearLevel(String appId)

# Obtener métricas completas en JSON
String getMetrics(String appId)
# Retorna: {"wearLevel":45,"launches":12,"activeMinutes":144,"reconstructions":2,...}

# Resetear desgaste (reconstrucción)
void resetWearLevel(String appId)

# Obtener contador de resets
int getReconstructions(String appId)
```

### Señales Disponibles

```python
# Se emite cuando cambia el desgaste
wearLevelChanged(String appId, int newLevel)

# Se emite cuando se resetea una app
wearLevelReset(String appId)
```

### Ejemplo de Uso (QDBus)

```bash
# Obtener desgaste de Firefox
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getWearLevel "firefox"

# Obtener métricas JSON
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getMetrics "firefox"

# Resetear
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.resetWearLevel "firefox"
```

---

## 🎨 Pipeline Visual (Shader)

### WearShader.qml - Efectos GLSL

```glsl
// Entrada: IconItem (renderizado a textura)
// Salida: Icono con efectos aplicados

vec4 wornIcon = originalIcon;

// EFECTO 1: Arañazos (scratches)
float noise = rand(texCoord × 100.0);
float scratches = step(0.98 - wearLevel × 0.1, noise);
wornIcon.rgb = mix(wornIcon.rgb, vec3(0.8), scratches × wearLevel);

// EFECTO 2: Desaturación/oscurecimiento
wornIcon.rgb *= (1.0 - wearLevel × 0.3);

// EFECTO 3: Animación Reset (spark inverso)
if (resetProgress > 0.0) {
    float sparkles = rand(texCoord × 200.0 + resetProgress);
    wornIcon.rgb = mix(wornIcon.rgb, vec3(1.0), sparkles × resetProgress);
}

gl_FragColor = wornIcon;
```

**Fases Visuales del Desgaste:**
- **0-25%:** Casi imperceptible, algunos rayones finos
- **25-50%:** Desgaste moderado, varios arañazos, ligeramente más oscuro
- **50-75%:** Muy desgastado, muchos rayones visibles, desaturado
- **75-100%:** Prácticamente destruido, muy oscuro, lleno de marcas

---

## ⚡ Optimizaciones Implementadas

### 1. **Timer Eficiente**
- Chequeo de apps activas cada **30 segundos** (no cada evento)
- Reduce overhead de actualizaciones constantemente

### 2. **Persistencia Lazy**
- Solo escribe a disco cuando hay cambios importantes
- Descarga de I/O comparado con guardar en cada evento

### 3. **Shader Optimizado**
- Cálculos en fragment shader (GPU, paralelo)
- Una sola pasada de rendering por frame
- No usa texturas externas, solo ruido procedural

### 4. **Lazy Loading de Configuración**
- Solo carga apps en m_appWearData si existen en config
- No inicializa toda la lista del sistema

---

## 🔄 Ciclo de Vida del Daemon

```
┌─────────────────────────────────────────┐
│ 1. Usuario inicia sesión KDE            │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│ 2. Daemon inicia (systemd/autostart)    │
│    - QCoreApplication::QCoreApplication │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│ 3. UsageTracker::UsageTracker()         │
│    - loadConfig()                       │
│    - startTimer(30000ms)                │
│    - Conecta a ActivityManager          │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│ 4. Registra servicio DBus               │
│    org.kde.iconwear/Tracker             │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│ 5. Event loop: app.exec()               │
│    Espera eventos DBus/ActivityManager  │
│    (estado: escuchando)                 │
└────────────────┬────────────────────────┘
                 │
           (Durante Sesión)
           ├─ Evento: app lanzada
           │  → onResourceOpened()
           │  → updateWearLevel()
           │  → Q_EMIT wearLevelChanged()
           │
           ├─ Timer cada 30s
           │  → checkActiveApplications()
           │  → updateWearLevel()
           │
           └─ Click derecho en Plasmoid
              → resetWearLevel()
              → Q_EMIT wearLevelReset()
                 │
                 ▼
┌─────────────────────────────────────────┐
│ 6. Cierre de sesión                     │
│    - saveConfig() (último guardado)     │
│    - Desconecta DBus                    │
│    - Destructor ~UsageTracker()         │
└─────────────────────────────────────────┘
```

---

## 🐛 Debugging y Troubleshooting

### Verificar que el daemon está corriendo

```bash
# Listar servicios DBus disponibles
qdbus | grep iconwear

# Si no aparece, ejecutar manualmente:
./build/bin/iconwear-daemon
```

### Ver logs en tiempo real

```bash
# Con Qt debug
QT_DEBUG_PLUGINS=1 ./build/bin/iconwear-daemon

# Con journalctl (si está en systemd)
journalctl -u iconwear-daemon -f
```

### Inspeccionar datos guardados

```bash
# Ver configuración actual
cat ~/.config/iconwearrc

# Limpiar todo (reset total)
rm ~/.config/iconwearrc
```

### Probar métodos DBus

```bash
# Llamar método getWearLevel
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getWearLevel "firefox"

# Llamar método getMetrics
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getMetrics "firefox"

# Escuchar señales
dbus-monitor "interface='org.kde.iconwear.Tracker'"
```

---

## 🚀 Futuras Mejoras

### Corto Plazo (Versión 0.2)
- [ ] Integración real de DBus con QML
- [ ] Configuración de factores de desgaste (UI)
- [ ] Gráfico de estadísticas por aplicación
- [ ] Auto-start del daemon

### Mediano Plazo (Versión 0.3)
- [ ] Diferentes tipos de desgaste (suciedad, grietas, óxido)
- [ ] Temas personalizables
- [ ] Export/Import de datos
- [ ] API REST para herramientas externas

### Largo Plazo (Versión 1.0)
- [ ] Sincronización entre dispositivos
- [ ] Integración con Global Shortcuts
- [ ] Plugin para gestores de ventanas alternativos
- [ ] Analytics anónimo de uso

---

## 📝 Licencia

MIT License © 2025 Nicolas Butterfield

---

## 📧 Contacto

**Autor:** Nicolas Butterfield  
**Email:** nicobutter@gmail.com  
**Proyecto:** https://github.com/lordcommander/iconwear-kde

---

**Última Actualización:** 18 de Diciembre de 2025
