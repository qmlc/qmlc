import AppType 1.0
import QtQuick 2.0

Item {
    id: root
    SubModel {
        id: extended
    }

    Component.onCompleted: {
        console.log("strprop: ", extended.strprop);
        extended.strprop = "Completed.";
        console.log("strprop: ", extended.strprop);
        console.log("intprop: ", extended.intprop);
        extended.intprop = 1;
        console.log("intprop: ", extended.intprop);
        extended.changeProperty("via invokable");
        console.log("extended: ", extended.text, extended.number);
        extended.text = "Altered";
        extended.number = 987;
        console.log("extended: ", extended.text, extended.number);
        // This should not succeed.
        //UcModel {}
    }
}
