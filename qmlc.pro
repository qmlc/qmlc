include(config.pri)

TEMPLATE = subdirs

SUBDIRS = \
	3rdparty \
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
