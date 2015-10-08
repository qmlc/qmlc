import AppType 1.0
import QtQuick 2.0

Item {
    id: root
    property int intValue: TwoModel.intprop
    property string strValue: OneModel.strprop

    onIntValueChanged: { console.log("intValue changed:", intValue) }
    onStrValueChanged: { console.log("strValue changed:", strValue) }

    Component.onCompleted: {
        console.log("strprop: ", strValue);
        OneModel.strprop = "Completed.";
        console.log("strprop: ", strValue);
        console.log("intprop: ", intValue);
        TwoModel.intprop = 1;
        console.log("intprop: ", intValue);
        OneModel.changeProperty("via invokable");
    }
}
