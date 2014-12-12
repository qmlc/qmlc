import QtQuick 2.2
import skytree.singleton.Lib 1.0

Item {
    property string foo: OnlyOne.textual;
    Component.onCompleted: {
        console.log("OnlyOne.textual, foo: ", OnlyOne.textual, foo);
        OnlyOne.textual = foo;
        console.log("OnlyOne.textual, foo: ", OnlyOne.textual, foo);
    }
}

