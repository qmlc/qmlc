
import QtQuick 2.2

Item {
    objectName: "i0"
    id: i0

    property int i1_x: i1_alias.x
    property alias i1_alias: i1
    property alias r0_height: r0.height

    Item {
        id: i1
        objectName: "i1"
        x: 111
    }

    Rectangle {
        id: r0
        objectName: "r0"
        height: 222
    }

    Item {
        id: i2
        objectName: "i2"
        property int i1_x: i1_alias.x
        property alias i1_alias: i1
        property alias r0_height: r0.height
    }
}
