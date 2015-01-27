import QtQuick 2.2
import "Lib"
import "."

Item {
    id: root

    User {
        id: first
    }

    User {
        id: second
    }

    Component.onCompleted: {
        first.foo = "first";
        second.foo = "second";
    }
}

