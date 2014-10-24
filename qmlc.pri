
# add CONFIG += qmc in .pro or qmake command before importing this file

# variables

# QMLC_QML: list the qml/js files to compile

# QMLC_DEST_DIR: This is the location that it will be installed to
# when make install is run.

# QMLC_EXTRA(optional): list extra files that has to be copied over(eg. qmldir_loader)

# QMLC_BASE_DIR(optional): Use this to set the base of the qmlc package if qmc
# libraries/includes aren't installed in standard locations.

# QMLC_EXIT_ON_ERROR(optional): Set to false if compiling should continue even
# if some file compilation failed. Default is true and compiling will exit as
# soon as an error occurs

# QMLC_QML2_IMPORT_PATH(optional): This will be prepended to the qml import
# path(QML2_IMPORT_PATH). Use if imported files are located in non standard
# directories

# QMLC_QML_BASE_DIR(optional): Set to base directory where qml/js is located or the
# current directory will be used

# QMLC_TMP_DEST_DIR(optional): Set this if there is a need to copy the QMLC_QML
# and QMLC_EXTRA files to another directory after compiling but before
# installing. This can be used where a plug-in is compiled and then is needed
# by another application when compiling it before the plug-in has been
# installed. Also if one wants to run a application and plug-ins without
# installing it this could be used.


isEmpty(QMLC_QML_BASE_DIR):QMLC_QML_BASE_DIR=.

INCLUDEPATH += $$QMLC_BASE_DIR/qmcloader
LIBS += -L$$QMLC_BASE_DIR/qmcloader
LIBS += -lqmcloader

# define var for $ so we can use it in QMAKE_POST_LINK to escape shell vars
DOLLAR = $

qmc {

    QMAKE_POST_LINK += export QML2_IMPORT_PATH=$$QMLC_QML2_IMPORT_PATH:$$QML2_IMPORT_PATH;

    basedir = $$OUT_PWD

    # go to directory where plug-in and qml is installed so paths work out
    !isEmpty(QMLC_QML_BASE_DIR){
        QMAKE_POST_LINK += cd $$QMLC_QML_BASE_DIR;
    }

    for(qmlfile, QMLC_QML) {

       qmcfile = $$replace(qmlfile, \\.qml, .qmc)
       qmcfile = $$replace(qmcfile, \\.js, .jsc)

       # compile
       QMAKE_POST_LINK += cd ./$$dirname(qmlfile);
       QMAKE_POST_LINK += echo + Compiling $$qmlfile;
       QMAKE_POST_LINK += qmc $$basename(qmlfile);

       !equals(QMLC_EXIT_ON_ERROR, "false"){
            QMAKE_POST_LINK += if [ $${DOLLAR}$? != 0 ]; then exit; fi;
       }

       # if QMLC_TMP_DEST_DIR is set we copy the compiled file to it. used with
       # plug-ins where the other code expect it in another directory to compile
       !isEmpty(QMLC_TMP_DEST_DIR){
           QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$_PRO_FILE_PWD_/$$qmcfile) $$QMLC_TMP_DEST_DIR/$$dirname(qmcfile)), /, $$QMAKE_DIR_SEP);
       }

       QMAKE_POST_LINK += cd -;

       # install
       target = install_$$lower($$basename(qmlfile))
       target = $$replace(target, \\.qml, _qmc)
       target = $$replace(target, \\.js, _jsc)
       path = $${target}.path

       #$$path = $$[QT_INSTALL_QML]/$$member(TARGETPATH, 0)
       $$path = $$[QT_INSTALL_QML]/$$QMLC_DEST_DIR
       commands = $${target}.commands
       $$commands += $$QMAKE_MKDIR $(INSTALL_ROOT)/$$QMLC_DEST_DIR/$$dirname(qmcfile);
       $$commands += $$QMAKE_COPY $$QMLC_QML_BASE_DIR/$$qmcfile $(INSTALL_ROOT)/$$QMLC_DEST_DIR/$$dirname(qmcfile)
       equals(QMLC_EXIT_ON_ERROR, "false"){
           $$commands += || echo;
       }else{
           $$commands += ;
       }
       INSTALLS += $$target
    }

    for(extrafile, QMLC_EXTRA) {
        !isEmpty(QMLC_TMP_DEST_DIR){
            QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$_PRO_FILE_PWD_/$$extrafile) $$QMLC_TMP_DEST_DIR/$$dirname(extrafile)), /, $$QMAKE_DIR_SEP);
        }
        target = install_$$lower($$basename(extrafile))
        path = $${target}.path
        #$$path = $$[QT_INSTALL_QML]/$$member(TARGETPATH, 0)
        $$path = $$[QT_INSTALL_QML]/$$QMLC_DEST_DIR
        commands = $${target}.commands
        $$commands += $$QMAKE_MKDIR $(INSTALL_ROOT)/$$QMLC_DEST_DIR/$$dirname(extrafile);
        $$commands += $$QMAKE_COPY $$extrafile $(INSTALL_ROOT)/$$QMLC_DEST_DIR/$$dirname(extrafile);
        INSTALLS += $$target
    }

    !isEmpty(QMLC_QML_BASE_DIR){
        QMAKE_POST_LINK += cd $$basedir;
    }

}
