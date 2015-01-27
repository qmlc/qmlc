import QtQuick 2.2
import "includer2.js" as Test2
import "includer3.js" as Test3
import "includer.js" as Test

Item {
    Component.onCompleted: {
        console.log("About to call...");
        console.log("completed: ", Test.test());
        console.log("completed2: ", Test2.test());
        console.log("completed3: ", Test3.test());
    }
}

