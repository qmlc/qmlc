QT += qml quick

RESOURCES = res.qrc res2.qrc res4.qrc
EXT_RES3 = res3.qrc
EXT_RES5 = res5.qrc

DEST_DIR=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

res3.target = res3.rcc
res3.depends = $$EXT_RES3 qml/External.qml
QMAKE_EXTRA_TARGETS += res3

res5.target = res5.rcc
res5.depends = $$EXT_RES5 qml/External.txt
QMAKE_EXTRA_TARGETS += res5
