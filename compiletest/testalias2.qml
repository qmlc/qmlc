
import QtQuick 2.2

Item {
    id: i0
    property alias color: r1.color
    Rectangle {
        id: r0
        objectName: "r0"
        width: 100
        height: 100
        color: i0.color
    }
    Rectangle {
        id: r1
        objectName: "r1"
        y: 100
        width: 100
        height: 100
        color: "#333333"
    }
}
