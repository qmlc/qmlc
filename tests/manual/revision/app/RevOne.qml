import AppTypeTest 1.1
import QtQuick 2.0

Revised {
    onIntpropChanged: {
        console.log("rev1 intprop changed, value now: ", intprop);
    }

    onR1propChanged: {
        console.log("rev1 r1prop changed, value now:", r1prop);
    }

    Component.onCompleted: {
        console.log("rev1 intprop: ", intprop);
        intprop = 11;
        console.log("rev1 intprop: ", intprop);
        console.log("rev1 r1prop: ", r1prop);
        r1prop = 11;
        console.log("rev1 r1prop: ", r1prop);
    }
}
