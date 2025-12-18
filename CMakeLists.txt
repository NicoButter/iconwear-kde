cmake_minimum_required(VERSION 3.16)

project(iconwear-kde)

set(KF_MIN_VERSION "5.90.0")
set(QT_MIN_VERSION "5.15.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    DBus
)

find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    I18n
    KIO
    Activities
    ActivitiesStats
)

add_subdirectory(src/daemon)
# add_subdirectory(src/plasmoid) # Plasmoids are usually installed differently, but we can manage it here

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
