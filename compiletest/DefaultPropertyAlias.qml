import QtQuick 2.2

Item {

    default property alias content: stack.children

    Item {
        id: stack
    }

    /*
    function test() {
        console.log(content.length)
    }
    Component.onCompleted: test()
    */
}
