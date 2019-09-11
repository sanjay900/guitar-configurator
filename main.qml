import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Universal 2.13
ApplicationWindow {
    id: applicationWindow
    visible: true
    Universal.theme: Universal.Dark
    Universal.accent: Universal.Violet

    StackView {
        id: mainStack
        initialItem: welcome
        anchors.fill: parent
        Welcome {
            id:welcome
        }
    }

}

















































































/*##^## Designer {
    D{i:0;autoSize:true;height:1080;width:1920}D{i:2;anchors_height:60;anchors_width:802;anchors_x:646;anchors_y:658}
D{i:1;anchors_height:200;anchors_width:200;anchors_x:846;anchors_y:87}
}
 ##^##*/