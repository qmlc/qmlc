

import QtQuick 2.0

QtObject {

    default property alias children: styleClass.__defaultPropertyFix
    property list<QtObject> __defaultPropertyFix: [Item {}] //QML doesn't allow an empty list here
}

