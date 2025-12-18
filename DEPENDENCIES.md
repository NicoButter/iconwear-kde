# Dependencias de IconWear KDE

Para compilar y ejecutar este proyecto en una distribución basada en KDE (como Kubuntu, Fedora KDE, Arch Linux, etc.), necesitarás instalar los siguientes paquetes de desarrollo.

## Herramientas de Construcción
- `cmake` (>= 3.16)
- `extra-cmake-modules` (ECM)
- `gcc` o `clang` (con soporte C++17)

## Bibliotecas de Qt 5 (>= 5.15)
- `qtbase5-dev` (Core, Gui, DBus)
- `qtdeclarative5-dev` (Qml, Quick)

## KDE Frameworks 5 (KF5)
- `libkf5config-dev` (KConfig)
- `libkf5i18n-dev` (KI18n)
- `libkf5kio-dev` (KIO)
- `libkf5activities-dev` (KActivities)
- `libkf5activitiesstats-dev` (KActivitiesStats)
- `libkf5plasma-dev` (Plasma Workspace)

## Comandos de instalación (Ejemplos)

### Ubuntu / Kubuntu / KDE Neon
```bash
sudo apt install build-essential cmake extra-cmake-modules \
    qtbase5-dev qtdeclarative5-dev \
    libkf5config-dev libkf5i18n-dev libkf5kio-dev \
    libkf5activities-dev libkf5activitiesstats-dev libkf5plasma-dev
```

### Fedora
```bash
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt5-qtbase-devel qt5-qtdeclarative-devel \
    kf5-kconfig-devel kf5-ki18n-devel kf5-kio-devel \
    kf5-kactivities-devel kf5-kactivitiesstats-devel kf5-plasma-devel
```

### Arch Linux
```bash
sudo pacman -S cmake extra-cmake-modules gcc \
    qt5-base qt5-declarative \
    kconfig ki18n kio kactivities kactivities-stats plasma-framework
```
