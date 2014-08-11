TEMPLATE = subdirs

SUBDIRS = \
	qmccompiler \
	qmcloader \
	qmc

qmc.depends = qmccompiler
