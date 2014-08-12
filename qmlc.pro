TEMPLATE = subdirs

SUBDIRS = \
	qmccompiler \
	qmcloader \
	qmc \
	compiletest \
	examples

qmc.depends = qmccompiler
compiletest.depends = qmccompiler qmcloader
examples.depends = qmccompiler qmcloader
