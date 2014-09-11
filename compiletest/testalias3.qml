
import QtQuick 2.2

Item {

    id: i0
    objectName: "i0"
    property int prop: 111

    Item {
        id: i1
    }

    Component {
        id: c0
        Item{
            id: i2
            objectName: "i2"
            property alias prop2: i3.prop2
            Item {
                id: i3
                objectName: "i3"
                property int prop2: 222
            }
        }
    }

    Loader { sourceComponent: c0 }

    Item {
        id: i4
        objectName: "i4"
        property alias prop: i0.prop
    }
}
