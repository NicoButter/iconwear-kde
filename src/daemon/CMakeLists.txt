add_executable(iconwear-daemon
    main.cpp
    usagetracker.cpp
)

target_link_libraries(iconwear-daemon
    Qt5::Core
    Qt5::DBus
    KF5::Activities
    KF5::ActivitiesStats
    KF5::ConfigCore
)

install(TARGETS iconwear-daemon ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
