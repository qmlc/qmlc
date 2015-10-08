#!/usr/bin/python

import os
import sys
import subprocess
import errno

perTypeArgs = 8

if (len(sys.argv) - 4) % perTypeArgs != 0:
    print "Usage: " + sys.argv[0] + "dir qt defines header class reg import major minor qmlname headers ..."
    print "  dir     : Directory to create."
    print "  qt      : Modules to add to QT in project file."
    print "  defines : Define options string for header preprocessing with moc."
    print "All of the following parameters are repeated for each type."
    print "  header  : Header file name where class is found."
    print "  class   : C++ class name in the header file."
    print "  reg     : Type of the registration to perform. One of these:"
    print "              reg: use qmlRegisterType"
    print "              reg,i,j,k,...: also use qmlRegisterRevision with i,..."
    print "              reg,uc: use qmlRegisterUncreatableType"
    print "              noreg: do not register, class needed for other reason"
    print "              na: use qmlRegisterTypeNotAvailable, ignores headers"
    print "              singleton: use qmlRegisterSingletonType with QObject"
    print "              singleton,callback: use qmlRegisterSingletonType with QObject"
    print "              jssingleton,callback: use qmlRegisterSingletonType with QJSValue"
    print "              QClassName: use qmlRegisterExtendedType"
    print "              QClassName,uc: use qmlRegisterExtendedUncreatableType"
    print "  import  : Import library name, from qmlRegisterType call."
    print "  major   : Major version number, from qmlRegisterType call."
    print "  minor   : Minor version number, from qmlRegisterType call."
    print "  qmlname : QML type name, from qmlRegisterType call."
    print "  headers : Comma-separated list of needed headers with paths."
    print "            Or, to use header and source file without processing it:"
    print "            direct:sourcefile.cpp[,source2.cpp,...]"
    print "Example: libdir \"core qml\" \"-DDEF=1 -DVAR\" mytype.h MyType reg MyLib 1 0 MyType QObject,subdir/someheader.h"
    exit(1)

targetDir = sys.argv[1].rstrip(os.sep)
qt = sys.argv[2]
defines = sys.argv[3]
argIndex = 4
head, proBase = os.path.split(targetDir)
if len(proBase) == 0:
    print "Error: directory to create amounts to empty directory name:" + sys.argv[1]
    exit(2)
if os.path.isabs(targetDir):
    print "Warning: directory to create is absolute: " + targetDir
    joined = targetDir
else:
    joined = os.path.join(os.getcwd(), targetDir)
relativeToParent = os.path.relpath(os.getcwd(), joined)
try:
    os.mkdir(targetDir)
except OSError, e:
    if e.errno != errno.EEXIST:
        print "Could not make target directory: " + targetDir
        print "Error: " + os.strerror(e.errno)
        exit(3)
try:
    os.chdir(targetDir)
except OSError, e:
    print "Failed to change directory to: " + targetDir
    print "Error: " + os.strerror(e.errno)
    exit(4)
mainName = "qmctypelib.cpp"
mainLines = []
mainBody = []
mainExtra = []
proName = proBase + ".pro"
proLines = [
    "TEMPLATE = lib",
    "CONFIG += dll" ]
if len(qt.strip()):
    proLines.append("QT += " + qt)
proLines.extend([
    "TARGET = " + proBase,
    "DEFINES += QMC _QMCTYPELIBBUILD",
    "SOURCES = " + mainName,
    "DEPENDPATH += " + relativeToParent,
    "INCLUDEPATH += " + relativeToParent ])
original2generated = {}
exitValue = 0

def commaSepFiles(prefix, commaSeparatedList):
    out = []
    if isinstance(commaSeparatedList, list):
        names = commaSeparatedList
    else:
        names = commaSeparatedList.split(",")
    for p in names:
        s = p.strip()
        if len(s) > 0:
            out.append(os.path.join(prefix, s));
    return out

def getSecond(original, separator):
    parts = original.split(separator)
    if len(parts) == 2:
        return parts[1].strip()
    return ""

def getDirectSources(includes):
    names = []
    for p in getSecond(includes, ":").split(","):
        if len(p.strip()):
            names.append(p.strip())
    return names

while argIndex + perTypeArgs <= len(sys.argv):
    header = sys.argv[argIndex]
    inputClass = sys.argv[argIndex + 1]
    regType = sys.argv[argIndex + 2]
    qmlImport = sys.argv[argIndex + 3]
    major = sys.argv[argIndex + 4]
    minor = sys.argv[argIndex + 5]
    qmlName = sys.argv[argIndex + 6]
    includes = sys.argv[argIndex + 7]
    argIndex += perTypeArgs
    if os.path.isabs(header):
        print "Warning: absolute path in header name: " + header
    direct = includes.startswith("direct:")
    directSources = getDirectSources(includes) if direct else []
    base = inputClass
    if not direct:
        base += "_" + major + "_" + minor
    elif len(directSources) == 0:
        print "Error: direct: requires file names: " + includes
        exitValue = 5
        continue
    original2generated[inputClass] = base
    # This needs further support for uncreatable types (really?) and singleton
    # types. Can I get away with dummy empty callback for singletons?
    # How about revisions? Types not available? Maybe not compilable?
    def makeArgs(qmlImport, major, minor, qmlName=None):
        args = '"' + qmlImport + '", ' + major + ', ' + minor
        if qmlName is not None:
            args += ', "' + qmlName + '"'
        return args
    skip = False
    common = makeArgs(qmlImport, major, minor, qmlName)
    if regType == "reg":
        reg = [ 'qmlRegisterType<' + base + '>(' + common + ')' ]
    elif regType == "reg,uc":
        reg = [ 'qmlRegisterUncreatableType<' + base + '>(' + common + ', "N/A")' ]
    elif regType.startswith("reg,"):
        revs = regType.split(",")
        reg = [ 'qmlRegisterType<' + base + '>(' + common + ')' ]
        for n in revs[1:]:
            reg.append('qmlRegisterRevision<' + base + ',' + n + '>(' + makeArgs(qmlImport, major, n) + ')')
    elif regType == "noreg":
        reg = [ ]
    elif regType == "na":
        reg = [ 'qmlRegisterTypeNotAvailable(' + common + ', "N/A")' ]
        skip = True
    elif regType.startswith("jssingleton,"):
        if not direct:
            print "Error: direct: must be used in includes with jssingleton"
            exitValue = 13
            continue
        func = getSecond(regType, ",")
        if len(func) == 0:
            print "Error: callback function name required: " + regType
            exitValue = 11
            continue
        # Callback declaration.
        mainExtra.append("QJSValue " + func + "(QQmlEngine*, QJSEngine*);")
        reg = [ 'qmlRegisterSingletonType(' + common + ', &' + func + ')' ]
        proLines.append("SOURCES += " + " ".join(commaSepFiles(relativeToParent, directSources)))
        skip = True
    elif regType.startswith("singleton,"):
        if not direct:
            print "Error: direct: includes must be used with singleton,callback"
            exitValue = 14
        func = getSecond(regType, ",")
        if len(func) == 0:
            print "Error: callback function name required: " + regType
            exitValue = 12
            continue
        mainExtra.append("QObject *" + func + "(QQmlEngine*, QJSEngine*);")
        reg = [ 'qmlRegisterSingletonType<' + base + '>(' + common + ', &' + func + ')' ]
    elif regType.startswith("singleton"):
        func = "cb_" + base;
        mainExtra.extend([
            "static QObject *" + func + "(QQmlEngine *e, QJSEngine *s) {",
            "  Q_UNUSED(e)",
            "  Q_UNUSED(s)",
            "  return new " + base + "();",
            "}" ])
        reg = [ 'qmlRegisterSingletonType<' + base + '>(' + common + ', &' + func + ')' ]
    else:
        parts = regType.split(",")
        # In case of extending a registered type, get the generated name.
        cn = original2generated.get(parts[0], parts[0])
        if len(parts) > 1 and parts[1] == "uc":
            reg = [ 'qmlRegisterExtendedUncreatableType<' + cn + ', ' + base + '>(' + common + ', "N/A")' ]
        else:
            reg = [ 'qmlRegisterExtendedType<' + cn + ', ' + base + '>(' + common + ')' ]
    for r in reg:
        mainBody.extend([
            '  if (' + r + ' == -1) {',
            '    qWarning() << "Failed: ' + r.replace('"', r'\"') + ')";',
            '    return false;',
            '  }' ])
    if skip:
        continue
    src_header = commaSepFiles(relativeToParent, header)[0]
    if direct:
        mainLines.append('#include "' + src_header + '"')
        srcs = commaSepFiles(relativeToParent, directSources)
        proLines.extend([
            "SOURCES += " + " ".join(srcs),
            "HEADERS += " + src_header ])
        continue
    mainLines.append('#include "' + base + '.h"')
    # Separate include headers and their paths.
    paths = []
    headers = []
    for inc in includes.split(","):
        inc = inc.strip()
        if len(inc) == 0:
            continue
        p, h = os.path.split(inc)
        if len(h):
            headers.append(h)
        else:
            print "Error: no file in header: " + inc
            exitValue = 6
        if len(p) and p != "." and p not in paths:
            paths.append(p)
            if os.path.isabs(p):
                print "Warning: absolute path in " + inc
    # Generate moc call and execute it.
    mocargs = [ "moc" ]
    for p in paths:
        mocargs.append("-I")
        mocargs.append(p)
    mocargs.extend(defines.split())
    moch = "moc_" + base + ".h"
    mocargs.extend([ "-E", src_header, "-o", moch ])
    val = subprocess.call(mocargs)
    if val != 0:
        print "Error in type info preprocessing, check the defines argument."
        print "Currently: " + defines
        print "Edit " + base + ".cpp target. Create files manually."
        exitValue = 7
    else:
        # Generate cpptype call and execute it.
        cppargs = [ "qmc-cpptypeplaceholder", "-i", moch, "-c", inputClass,
            "--source", base + ".cpp", "--header", base + ".h",
            "--class", base ]
        cppargs.extend([ "-b", ",".join(headers) ])
        p, h = os.path.split(header)
        cppargs.extend([ "-d", h ])
        val = subprocess.call(cppargs)
        if val != 0:
            print "Error in placeholder source generation."
            print "Edit " + base + ".cpp target. Create files manually."
            exitValue = 8
    proLines.extend([
        base + ".commands = " + " ".join(cppargs),
        base + ".target = " + base + ".cpp",
        base + ".depends = moc_" + base + ".cpp",
        "QMAKE_EXTRA_TARGETS += " + base,
        "SOURCES += " + base + ".cpp",
        "HEADERS += " + base + ".h" ])

mainLines.extend([
    "#include <QtCore/QtGlobal>",
    "#include <QtQml>",
    "#include <QDebug>" ])
mainLines.extend(mainExtra)
mainLines.append('extern "C" bool registerQmlTypes() {')
mainLines.extend(mainBody)
mainLines.extend([
    "  return true;",
    "}" ])

def writeFile(name, lines):
    try:
        f = open(name, "w")
        for line in lines:
            f.write(line + "\n")
        f.close()
    except Exception, e:
        print "Error: failed to write: " + name
        print e
        return False
    return True

if not writeFile(proName, proLines):
    exitValue = 9
if not writeFile(mainName, mainLines):
    exitValue = 10
exit(exitValue)
