
TARGET = qmapwatchlet
TEMPLATE = lib
# CONFIG   += plugin
CONFIG   += mobility

SOURCES += qmapwatchletplugin.cpp qmapwatchlet.cpp

HEADERS += qmapwatchletplugin.h qmapwatchlet.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsowatch/release/ -lsowatch
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsowatch/debug/ -lsowatch
else:symbian: LIBS += -lsowatch
else:unix: LIBS += -L$$OUT_PWD/../libsowatch/ -lsowatch

INCLUDEPATH += $$PWD/../libsowatch
DEPENDPATH += $$PWD/../libsowatch

unix:!symbian {
	maemo5 {
		target.path = /opt/sowatch/watchlets
	} else {
		target.path = /usr/lib/sowatch/watchlets
	}
	INSTALLS += target
}

OTHER_FILES += \
    metawatch-digital.qml

RESOURCES += \
    qmapwatchlet.qrc















