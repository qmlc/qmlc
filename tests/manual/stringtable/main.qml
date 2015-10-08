import QtQuick 2.2

Item {
    SomeType {
        id: foo
        states: State {
            name: "changed"
            PropertyChanges {
                target: foo
                value: -1
                // Must not be visible in qmcdump output.
            }
        }
    }
    Connections {
        target: foo
        onValueChanged: {
            console.log("Must not be visible in qmcdump output:", foo.value)
        }
    }
    ListModel {
        id: customParsed
        ListElement {
            name: "Element 1"
            weight: Font.Light
        }
        ListElement {
            name: "Element 2"
            weight: Font.Normal
        }
        ListElement {
            name: "Element 3"
            weight: Font.DemiBold
        }
    }
    Component.onCompleted: {
        console.log("altering value:", foo.value);
        foo.value = 1;
        console.log("altered value:", foo.value);
        foo.state = "changed";
    }
}

