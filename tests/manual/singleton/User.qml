import QtQuick 2.2
import "Lib" 1.0

Item {
    property string foo: OnlyOne.textual;
    Component.onCompleted: {
        console.log("OnlyOne.textual, foo: ", OnlyOne.textual, foo);
        OnlyOne.textual = foo;
        console.log("OnlyOne.textual, foo: ", OnlyOne.textual, foo);
    }
}

