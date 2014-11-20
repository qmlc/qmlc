import QtQuick 2.2

import "testscriptref.js" as Handle

Item {
   Item {
        id: i1
        width: 249
   }

   Connections {
        objectName: "connections"
        target: Handle.test(i1)
    }

}
