
// always to include
#include <QCoreApplication>
#include <QtQml>
#include "qrccompiler.h"

// include program specific cpp types
#include "piechart.h"
#include "pieslice.h"

int main(int argc, char **argv)
{
    // register all cpp types
    qmlRegisterType<PieChart>("Charts", 1, 0, "PieChart");
    qmlRegisterType<PieSlice>("Charts", 1, 0, "PieSlice");

    // compile all possible files in the qrc in this directory and put in the
    // third args(dir)
    QrcCompiler qrccompiler;
    return qrccompiler.compile(argc, argv, "../import", "res.qrc");
}

