import QtQuick 2.2
import skytree.singleton.Lib 1.0
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

