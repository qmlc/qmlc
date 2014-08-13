TEMPLATE = subdirs

SUBDIRS = \
	compile \
	app

app.depends = compile
