import AppTypeTest 1.0
import QtQuick 2.0

Item {
    ModelTest {
        SubModel {
            text: "set_def";
            number: 54321;
            onTextChanged: { console.log("text changed:", text); }
            onNumberChanged: { console.log("number changed:", number); }
        }
        id: test

        onStrpropChanged: {
            console.log("New strprop value via parameter: ", newValue);
        }

        onIntpropChanged: {
            console.log("intprop changed, value now: ", intprop);
        }

        onSubChanged: {
            console.log("sub changed: now:", sub.text, sub.number);
        }
    }
    SimpleModel {
        id: direct
        onIntpropChanged: {
            console.log("direct intprop changed, value now: ", intprop);
        }
    }

    Component.onCompleted: {
        console.log("strprop: ", test.strprop);
        test.strprop = "Completed.";
        console.log("strprop: ", test.strprop);
        console.log("intprop: ", test.intprop);
        test.intprop = 1;
        console.log("intprop: ", test.intprop);
        test.changeProperty("via invokable");
        console.log("sub: ", test.sub.text, test.sub.number);
        test.sub.text = "Altered";
        test.sub.number = 987;
        console.log("sub: ", test.sub.text, test.sub.number);
        direct.intprop = 5;
    }
    // These will cause run-time error and fail QML compilation.
    //UcModel {
    //    id: foo;
    //    number: 13;
    //}
    //NaModel {
    //    id: name
    //}
}
