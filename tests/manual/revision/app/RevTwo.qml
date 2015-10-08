import AppTypeTest 1.2
import QtQuick 2.0

Revised {
    onIntpropChanged: {
        console.log("rev2 intprop changed, value now: ", intprop);
    }

    onR1propChanged: {
        console.log("rev2 r1prop changed, value now:", r1prop);
    }

    onR2propChanged: {
        console.log("rev2 r2prop changed, value now:", r2prop);
    }

    Component.onCompleted: {
        console.log("rev2 intprop: ", intprop);
        intprop = 22;
        console.log("rev2 intprop: ", intprop);
        console.log("rev2 r1prop: ", r1prop);
        r1prop = 22;
        console.log("rev2 r1prop: ", r1prop);
        console.log("rev2 r2prop: ", r2prop);
        r2prop = 22;
        console.log("rev2 r2prop: ", r2prop);
    }
}
