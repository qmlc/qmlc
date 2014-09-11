include(config.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
	3rdparty \
	qmccompiler \
	qmcloader \
	qmc \
    tools \
	compiletest \
	examples \
	tests

qmc.depends = qmccompiler
compiletest.depends = qmccompiler qmcloader
examples.depends = qmccompiler qmcloader
tests.depends = qmccompiler qmcloader
