import QtQuick 2.0

Rectangle {
    width: 200
    height: 200
    Component.onCompleted: {
        console.log("External completed.")
    }
}
