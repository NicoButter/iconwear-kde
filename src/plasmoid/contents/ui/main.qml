import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

Item {
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    property string appId: "org.kde.dolphin.desktop"
    property real wearLevel: 0.0

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

    ColumnLayout {
        anchors.fill: parent
        
        Item {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            Layout.alignment: Qt.AlignCenter

            PlasmaCore.IconItem {
                id: iconItem
                anchors.fill: parent
                source: "system-file-manager"
                visible: false // Hide original to show shader version
            }

            WearShader {
                anchors.fill: parent
                source: iconItem
                wearLevel: parent.parent.parent.wearLevel / 100.0
            }
        }

        PlasmaComponents.Label {
            text: "Wear: " + Math.round(parent.parent.wearLevel) + "%"
            Layout.alignment: Qt.AlignCenter
        }
        
        PlasmaComponents.Slider {
            from: 0
            to: 100
            value: parent.parent.wearLevel
            onMoved: parent.parent.wearLevel = value
            Layout.fillWidth: true
        }
    }
}
