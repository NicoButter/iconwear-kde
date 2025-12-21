import QtQuick 2.15
import QtQuick.Window 2.15
import "src/plasmoid/contents/ui" as IW

Window {
    visible: true
    width: 200
    height: 200
    title: "WearShader test"

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        ShaderEffectSource {
            id: srcItem
            anchors.centerIn: parent
            sourceItem: Image {
                source: "/usr/share/icons/hicolor/64x64/apps/system-file-manager.png"
                fillMode: Image.PreserveAspectFit
                width: 64
                height: 64
            }
            live: true
        }

        IW.WearShader {
            anchors.fill: parent
            source: srcItem
            wearLevel: 0.5
        }
    }
}
