include(config.pri)

TEMPLATE = subdirs

SUBDIRS = \
	qmccompiler \
	qmcloader \
	qmc \
	compiletest \
	examples \
	tests

qmc.depends = qmccompiler
compiletest.depends = qmccompiler qmcloader
examples.depends = qmccompiler qmcloader
tests.depends = qmccompiler qmcloader
