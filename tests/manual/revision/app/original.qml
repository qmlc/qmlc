import AppTypeTest 1.0
import QtQuick 2.0

Item {
    Revised {
        id: test

        onIntpropChanged: {
            console.log("intprop changed, value now: ", intprop);
        }
    }

    RevOne {
        id: rev1
    }

    RevTwo {
        id: rev2
    }

    Component.onCompleted: {
        console.log("intprop: ", test.intprop);
        test.intprop = 1;
        console.log("intprop: ", test.intprop);
    }
}
