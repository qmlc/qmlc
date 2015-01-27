import QtQuick 2.2
import skytree.toolkit.core 2.0
import "." 1.0

Item {
    SkytreeCore {
        id: skytree_core
    }
    Stuff {
        id: name
    }
    Button {
        platformStyle: ButtonStyle {
            background: "image://theme/call-button-background-red"
            pressedBackground: "image://theme/call-button-background-red-pressed"
        }
    }
    Component.onCompleted: { console.log("Done."); }
}

