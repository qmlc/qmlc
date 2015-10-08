#!/usr/bin/python

# Used to sort statement output when qmc is built with DEFINES+=DEBUG_QMC and
# --debug option has been given on command-line.

import os
import sys
import string
import re

if len(sys.argv) < 2 or len(sys.argv) > 3:
    print "Usage: %s filename [-q]" % os.path.basename(sys.argv[0])
    print "  filename: qmc --debug output statement file."
    print "  -q      : Do not print qobject<> type strings."
    exit(1)

suppress_qobject = "-q" in sys.argv

groups = []
f = open(sys.argv[1], "r")
for line in f.readlines():
    if line[0:2] == "0 ":
        groups.append([])
    groups[-1].append(line)
f.close()

groups.sort(key=lambda x: x[0] + str(len(x)))
for g in groups:
    for line in g:
        if suppress_qobject:
            line = re.sub("qobject<[a-zA-Z_0-9]*?>", "qobject<>", line)
        sys.stdout.write(line)
