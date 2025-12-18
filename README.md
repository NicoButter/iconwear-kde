# IconWear KDE

**Los íconos envejecen según cuánto los usás. Tu escritorio cuenta tu historia.**

Una aplicación para KDE Plasma que añade una dimensión visual única: **los iconos se desgastan progresivamente según su uso**, mostrando arañazos, desaturación y pérdida de brillo. Puedes "restaurarlos" resetando su apariencia con una animación visual satisfactoria.

<div align="center">

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![KDE](https://img.shields.io/badge/KDE-Plasma-0051BA.svg)](https://www.kde.org/)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)
[![Python](https://img.shields.io/badge/Status-Active%20Development-brightgreen)]()

### 🎨 Logo del Proyecto

![IconWear Logo](./assets/logo.png)

</div>

---

## ✨ Características

- **Desgaste Visual Progresivo:** Los iconos mostran arañazos, desaturación y oscurecimiento proporcional al uso
- **Desgaste Ponderado Realista:** Fórmula que considera tanto lanzamientos como tiempo activo
- **Interfaz DBus Completa:** API remota para integración con otras herramientas
- **Persistencia Automática:** Los datos se guardan en `~/.config/iconwearrc`
- **Shader GLSL en GPU:** Efectos visuales renderizados en tiempo real
- **Animación de Reconstrucción:** Reset visual satisfactorio con efecto "spark"
- **Menú Contextual:** Opciones para ver métricas y restaurar iconos

---

## 📸 Capturas de Pantalla

### Interfaz Principal
![Screenshot 1](./assets/screenshot_1.png)

### Desgaste Visual en Iconos
![Screenshot 2](./assets/screenshot_2.png)

### Widget de Plasma con Tooltip
![Screenshot 3](./assets/screenshot_3.png)

## 📸 Capturas de Pantalla

### Interfaz Principal
![Screenshot 1](./assets/screenshot_1.png)

### Desgaste Visual en Iconos
![Screenshot 2](./assets/screenshot_2.png)

### Widget de Plasma
![Screenshot 3](./assets/screenshot_3.png)

---

## 🚀 Inicio Rápido

### 1️⃣ Instalación de Dependencias

#### Ubuntu / Kubuntu / KDE Neon
```bash
sudo apt install build-essential cmake extra-cmake-modules \
    qtbase5-dev qtdeclarative5-dev \
    libkf5config-dev libkf5i18n-dev libkf5kio-dev \
    libkf5activities-dev libkf5activitiesstats-dev libkf5plasma-dev
```

#### Fedora
```bash
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt5-qtbase-devel qt5-qtdeclarative-devel \
    kf5-kconfig-devel kf5-ki18n-devel kf5-kio-devel \
    kf5-kactivities-devel kf5-kactivitiesstats-devel kf5-plasma-devel
```

#### Arch Linux
```bash
sudo pacman -S cmake extra-cmake-modules gcc \
    qt5-base qt5-declarative \
    kconfig ki18n kio kactivities kactivities-stats plasma-framework
```

### 2️⃣ Compilación e Instalación

```bash
git clone https://github.com/tu-usuario/iconwear-kde.git
cd iconwear-kde

mkdir build && cd build
cmake ..
make
sudo make install

# Reinicia la sesión de Plasma o recarga el daemon
killall iconwear-daemon
kquitapp5 plasmashell
```

---

## 📊 Fórmula de Desgaste

IconWear utiliza un **sistema de desgaste ponderado realista** que considera:

```
DESGASTE = (lanzamientos × 1.0) + (minutos_activos × 0.01)
         = Normalizado 0-100
```

**Ejemplos:**
- VS Code abierto 50 veces → Desgaste ≈ 50
- Firefox abierto 2 veces, usado 2 días → Desgaste ≈ 30.8
- Editor abierto 100 veces, 1 min promedio → Desgaste ≈ 101 (capped a 100)

**¿Por qué es mejor?**
- ✅ Más realista: apps usadas todo el día vs. apps lanzadas muchas veces brevemente
- ✅ Elegante: pondera ambas métricas de forma coherente
- ✅ Justa: el desgaste visual refleja el uso real

---

## 🏗️ Arquitectura del Sistema

### Componentes Principales

```
┌─────────────────────────────────────────┐
│     KDE Plasma Desktop (UI Frontend)    │
├─────────────────────────────────────────┤
│  Plasmoid Widget                        │
│  ├─ main.qml (UI y lógica)             │
│  └─ WearShader.qml (Efectos GLSL)      │
└────────────┬────────────────────────────┘
             │ DBus API
             ▼
┌─────────────────────────────────────────┐
│   iconwear-daemon (Backend Service)     │
├─────────────────────────────────────────┤
│  UsageTracker                           │
│  ├─ Rastrea lanzamientos               │
│  ├─ Acumula tiempo activo              │
│  ├─ Calcula desgaste ponderado         │
│  └─ Persiste en ~/.config/iconwearrc   │
└────────────┬────────────────────────────┘
             │
             ▼
    KDE ActivityManager
    (System Events)
```

### Flujo de Datos

1. **Usuario abre aplicación** → ActivityManager emite `ResourceOpened`
2. **Daemon recibe evento** → Incrementa `launches` y registra tiempo activo
3. **Calcula desgaste** → Usa fórmula ponderada
4. **Emite señal DBus** → `wearLevelChanged(appId, newLevel)`
5. **Plasmoid recibe** → Actualiza visual del shader
6. **Icono cambia** → Muestra más desgaste (arañazos + oscurecimiento)

---

## 🌐 Interfaz DBus

El daemon expone una API completa via DBus:

```
Servicio:  org.kde.iconwear
Ruta:      /Tracker
Interfaz:  org.kde.iconwear.Tracker
```

### Métodos Disponibles

```bash
# Obtener desgaste (0-100)
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getWearLevel "firefox"

# Obtener métricas completas (JSON)
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getMetrics "firefox"

# Resetear desgaste
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.resetWearLevel "firefox"

# Obtener contador de resets
qdbus org.kde.iconwear /Tracker org.kde.iconwear.Tracker.getReconstructions "firefox"
```

### Señales

```bash
# Escuchar cambios de desgaste en tiempo real
dbus-monitor "interface='org.kde.iconwear.Tracker'"
```

---

## 🗂️ Estructura del Proyecto

```
iconwear-kde/
├── src/
│   ├── daemon/                      # Backend (systemd service)
│   │   ├── main.cpp                 # Punto de entrada, registra DBus
│   │   ├── usagetracker.h           # Header con interfaz pública
│   │   └── usagetracker.cpp         # Implementación del core
│   │
│   └── plasmoid/                    # Frontend (Plasma Widget)
│       ├── metadata.json            # Metadatos del widget
│       └── contents/ui/
│           ├── main.qml             # UI principal
│           └── WearShader.qml       # Shader GLSL de efectos
│
├── CMakeLists.txt                   # Build config
├── README.md                        # Este archivo (completo)
├── ARCHITECTURE.md                  # Guía técnica detallada
├── DEPENDENCIES.md                  # Requisitos del sistema
├── .gitignore
└── LICENSE (MIT)
```

---

## 💡 Cómo Funciona Técnicamente

### 1. Rastreo de Uso

El daemon escucha eventos de `KActivities::ActivityManager`:

```cpp
// Cuando usuario abre app
onResourceOpened(activity, agent, resource)
  → Incrementa launches++
  → Marca app como activa
  → Recalcula desgaste
  → Emite wearLevelChanged()
```

### 2. Cálculo de Desgaste

Cada 30 segundos, verifica apps activas:

```cpp
checkActiveApplications()
  → Detecta apps cerradas (inactividad > 5 min)
  → Acumula tiempo activo
  → Recalcula: desgaste = (launches × 1.0) + (minutos × 0.01)
  → Persiste a KConfig
```

### 3. Visualización GLSL

El shader aplica efectos en tiempo real:

```glsl
// Ruido procedural: scratches/arañazos
scratch = rand(texCoord × 100.0) × wearLevel

// Desaturación: oscurecimiento
color.rgb *= (1.0 - wearLevel × 0.3)

// Reset: destello blanco brillante
spark = mix(color, vec3(1.0), resetProgress)
```

### 4. Persistencia

Datos guardados en `~/.config/iconwearrc`:

```ini
[Applications/firefox]
wearLevel=45
launches=12
activeTimeSeconds=8640
reconstructions=2
```

---

## 🎮 Uso del Widget

### Visualización

- **Hover sobre icono** → Muestra tooltip con estadísticas:
  - Aperturas: número de lanzamientos
  - Desgaste: porcentaje visual
  - Activo: minutos totales
  - Restauraciones: contador de resets

### Menú Contextual (Clic Derecho)

- **Ver Métricas** → Toggle del tooltip
- **Restaurar Icono** → Reset con animación spark

### Animación de Reset

Cuando restauras un icono:
1. Efecto de "destello" blanco brillante (300ms)
2. Desvanecimiento suave (150ms)
3. Contador de restauraciones +1
4. Desgaste vuelve a 0

---

## 🐛 Debugging y Troubleshooting

### Verificar que el daemon está corriendo

```bash
qdbus | grep iconwear
```

### Ver logs en tiempo real

```bash
QT_DEBUG_PLUGINS=1 ./build/bin/iconwear-daemon
```

### Inspeccionar datos guardados

```bash
cat ~/.config/iconwearrc
```

### Limpiar todo (reset total)

```bash
rm ~/.config/iconwearrc
killall iconwear-daemon
```

---

## 📋 Requisitos Completos

### Herramientas de Construcción
- `cmake` ≥ 3.16
- `extra-cmake-modules` (ECM)
- `gcc` o `clang` con soporte C++17
- `make`

### Librerías Qt 5
- `qtbase5-dev` (Core, Gui, DBus)
- `qtdeclarative5-dev` (QML, Quick)

### KDE Frameworks 5
- `libkf5config-dev` (Configuración)
- `libkf5i18n-dev` (Internacionalización)
- `libkf5kio-dev` (I/O)
- `libkf5activities-dev` (Actividades del sistema)
- `libkf5activitiesstats-dev` (Estadísticas de actividades)
- `libkf5plasma-dev` (Framework de Plasma)

---

## 🗺️ Roadmap

### Versión 0.2 (Próximo)
- [ ] Integración real de DBus con QML
- [ ] Panel de configuración de factores de desgaste
- [ ] Gráfico de estadísticas por aplicación

### Versión 0.3
- [ ] Diferentes tipos de desgaste visual (suciedad, grietas, óxido)
- [ ] Temas personalizables
- [ ] Export/Import de datos

### Versión 1.0
- [ ] Sincronización entre dispositivos
- [ ] Global Shortcuts
- [ ] Plugin para WMs alternativos

---

## 📚 Documentación Adicional

Para información técnica detallada, consulta:
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Diseño arquitectónico completo, diagramas, ciclo de vida
- **[DEPENDENCIES.md](DEPENDENCIES.md)** - Requisitos por distribución (Fedora, Ubuntu, Arch)
- Comentarios Doxygen en código fuente para desarrolladores

---

## � Archivos Fuente Documentados

Todos los archivos están documentados siguiendo estándares Doxygen:

- **`src/daemon/main.cpp`** - Punto de entrada del daemon con documentación DBus
- **`src/daemon/usagetracker.h`** - Interfaz pública con comentarios Doxygen
- **`src/daemon/usagetracker.cpp`** - Implementación con algoritmos explicados
- **`src/plasmoid/contents/ui/main.qml`** - UI documentada con flujo de interacción
- **`src/plasmoid/contents/ui/WearShader.qml`** - Shader GLSL con fórmulas matemáticas

---

## 💬 Preguntas Frecuentes

### ¿Los datos de desgaste se sincronizan entre dispositivos?
No en esta versión. Los datos se guardan localmente en `~/.config/iconwearrc`. Esto está planeado para v1.0.

### ¿Puedo cambiar la velocidad de desgaste?
Actualmente los factores están hardcodeados. Habrá panel de configuración en v0.2.

### ¿Funciona en Wayland?
Debería funcionar pero no ha sido testeado. El proyecto está optimizado para X11 por ahora.

### ¿Hay impacto en rendimiento?
Mínimo: el daemon usa un timer cada 30 segundos y el shader se ejecuta en GPU.

### ¿Puedo contribuir?
¡Claro! Abre un issue primero para discutir cambios importantes.

---

## 🎯 Objetivos del Proyecto

IconWear busca:
1. **Añadir dimensión temporal al escritorio** - Los iconos cuentan historias de uso
2. **Ser elegante pero funcional** - Bonito visual sin sacrificar utilidad
3. **Facilmente extensible** - API DBus abierta para integraciones
4. **Respeta privacidad** - Datos 100% locales, sin tracking externo

---

## 👨‍💻 Autor

**Nicolas Butterfield**  
📧 [nicobutter@gmail.com](mailto:nicobutter@gmail.com)

---

## 📞 Contacto & Soporte

Para reportar bugs o sugerir features, abre un issue en GitHub.

---

## 📄 Licencia

Este proyecto está bajo la licencia **MIT**. Consulta el archivo [LICENSE](LICENSE) para más detalles.

```
MIT License © 2025 Nicolas Butterfield

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

---

## 🙏 Agradecimientos

- KDE Project por los frameworks excelentes
- Qt Project por el engine de aplicaciones
- Comunidad open-source por inspiración

---

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Para cambios importantes, abre un issue primero para discutir qué te gustaría cambiar.

**Última Actualización:** 18 de Diciembre de 2025  
**Versión:** 0.1 (Alpha)  
**Estado:** 🟢 Active Development
