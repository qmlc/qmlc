include(../config.pri)

TEMPLATE=lib

INSTALL_HEADERS=$$system("find . -name '*.h' -o -name '*pri'")

for(header, INSTALL_HEADERS) {
  hpath = $$INCLUDEDIR/qmcloader/3rdparty/$${dirname(header)}
  eval(headers_$${hpath}.files += $$header)
  eval(headers_$${hpath}.path = $$hpath)
  eval(INSTALLS *= headers_$${hpath})
}
