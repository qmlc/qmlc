pragma Singleton
import QtQuick 2.2

QtObject {
    id: single
    property string textual: "Initial String"

    Component.onCompleted: {
        console.log("OnlyOne completed.");
    }
}

