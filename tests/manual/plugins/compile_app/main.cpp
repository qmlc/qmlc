
// always to include
#include <QCoreApplication>
#include <QtQml>
#include "qrccompiler.h"

// include program specific cpp types

int main(int argc, char **argv)
{
    // register all cpp types

    // compile all possible files in the qrc in this directory and put in the
    // third args(dir)
    QrcCompiler qrccompiler;
    return qrccompiler.compile(argc, argv, "../", "app.qrc");
}

