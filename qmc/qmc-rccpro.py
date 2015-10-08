#!/usr/bin/python

import sys

if len(sys.argv) < 7:
    print "Usage: " + sys.argv[0] + "pri-file in-dir out-dir qrc-var deps-var qmc-flags qrc-files..."
    print "  pri-file : File to create. To be included in calling project file."
    print "  in-dir   : Source directory. Use $$PWD."
    print "  out-dir  : Output directory. Use $$OUT_PWD."
    print "  qrc-var  : Variable to which to add qrc-files."
    print "  deps-var : Variable to which to add dependency file list."
    print "             Pass \\\" \\\" if none."
    print "  qmc-flags: Command-line options for qmc. Pass \\\" \\\" if none."
    print "  qrc-files: Names of the qrc files to process."
    print "Example: __qmc-res.pri $$PWD $$OUT_PWD RESOURCES \\\" \\\" \"-g \" res.qrc res2.qrc"
    print "Example: __qmc-res.pri $$PWD $$OUT_PWD RESVAR DEPSVAR \\\" \\\" res.qrc"
    exit(1)

import os
import time
import subprocess
import xml.etree.ElementTree as et

outName = sys.argv[1].strip()
inDir = sys.argv[2].strip()
outDir = sys.argv[3].strip()
varName = sys.argv[4].strip()
depName = sys.argv[5].strip()
qmcFlags = sys.argv[6].strip()
qrcFiles = sys.argv[7:]

toInFromOut = os.path.relpath(inDir, outDir)
if toInFromOut == ".":
    toInFromOut = ""

if len(outName) == 0 or len(varName) == 0:
    print "Error: pri-name or variable must not be empty"
    exit(6)

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

def qmcCommand(name, asList=False):
    c = "qmc " + qmcFlags + " " + name
    if not asList:
        return c
    out = []
    for p in c.split(" "):
        if len(p):
            out.append(p)
    return out

def addOutVar(name, first=False):
    outPri.append(varName + (" = " if first else " += ") + name)

tgtNum = 0
def targetName():
    global tgtNum
    tgtNum += 1
    return "__qmc_" + varName + str(tgtNum)

rootDir = os.getcwd()

outPri = [ "QMAKE_DISTCLEAN += " + outName ]
addOutVar("", True)
if len(depName) != 0:
    outPri.append(depName + " =")
outData = set()
touchables = []
for res in qrcFiles:
    # Read source file.
    try:
        qrc = et.parse(res)
    except Exception, e:
        print e
        print "Error: failed to read: " + res
        exit(4)
    toHere, n = os.path.split(res)
    name = os.path.join(toHere, "__qmc_" + n)
    if len(toHere):
        os.chdir(toHere)
    # Shadow build requires relative path to work.
    toHere = os.path.join(toInFromOut, toHere)
    changed = False
    deps = []
    for elem in qrc.iter():
        if elem.tag != "file":
            continue
        source = ""
        target = ""
        if elem.text.endswith(".js"):
            source = elem.text
            elem.text += "c"
            target = elem.text
            if "alias" in elem.attrib:
                elem.attrib["alias"] = elem.attrib["alias"] + "c"
            changed = True
        elif elem.text.endswith(".qml"):
            source = elem.text
            elem.text = elem.text[:-1] + "c"
            target = elem.text
            if "alias" in elem.attrib:
                elem.attrib["alias"] = elem.attrib["alias"][:-1] + "c"
            changed = True
        else:
            deps.append(os.path.join(toHere, elem.text))
            continue
        deps.append(os.path.join(toHere, elem.text))
        src = os.path.join(toHere, source)
        outData.add((src, os.path.join(toHere, target),))
        if not os.path.isfile(target):
            touchables.append(src)
            open(target, "w").close()
    os.chdir(rootDir)
    if changed:
        try:
            qrc.write(name)
        except Exception, e:
            print e
            print "Error: failed to write modified " + res + " to " + name
            exit(2)
        addOutVar(name)
        outPri.append("QMAKE_DISTCLEAN += " + name)
        tgtFile = name
    else:
        addOutVar(res)
        tgtFile = outName
    tgt = targetName()
    outPri.extend([
        tgt + ".target = " + tgtFile,
        tgt + ".depends = " + res,
        tgt + '.commands = echo && echo "Resource list file changed. Re-run qmake" && echo && exit 1',
        "QMAKE_EXTRA_TARGETS += " + tgt ])
    if len(depName):
        for d in deps:
            outPri.append(depName + " += " + d)

for src, dst in outData:
    tgt = targetName()
    outPri.extend([
        tgt + ".target = " + dst,
        tgt + ".depends = " + src,
        tgt + ".commands = " + qmcCommand(src),
        "QMAKE_CLEAN += " + dst,
        "QMAKE_EXTRA_TARGETS += " + tgt ])

if not writeFile(outName, outPri):
    exit(3)

# Ensure that file times differ.
time.sleep(1)
for t in touchables:
    if not os.path.isfile(t):
        print "Warning: missing qrc source: " + t
        continue
    cmd = [ "touch", t ]
    if subprocess.call(cmd) != 0:
        print "Error touching source: " + " ".join(cmd)
        exit(5)
