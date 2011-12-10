TEMPLATE = app

TARGET = sowatchd

QT       += core gui
CONFIG   -= app_bundle

# Qt Mobility 1.2
maemo5 {
	CONFIG += mobility12
} else {
	CONFIG += mobility
}
MOBILITY += serviceframework

SOURCES += main.cpp daemon.cpp service.cpp

HEADERS += global.h daemon.h service.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsowatch/release/ -lsowatch
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsowatch/debug/ -lsowatch
else:symbian: LIBS += -lsowatch
else:unix: LIBS += -L$$OUT_PWD/../libsowatch/ -lsowatch

INCLUDEPATH += $$PWD/../libsowatch
DEPENDPATH += $$PWD/../libsowatch

unix {
	!isEmpty(MEEGO_VERSION_MAJOR)|maemo5 {
		QMAKE_RPATHDIR += /opt/sowatch/lib
		target.path = /opt/sowatch/bin
	} else {
		target.path = /usr/bin
	}
	INSTALLS += target
}

xml.path = /opt/sowatch/xml
xml.files = service.xml
dbus.path = /usr/share/dbus-1/services
dbus.files = com.javispedro.sowatch.service.sowatch-service.service
INSTALLS += xml dbus
