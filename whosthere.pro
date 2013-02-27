QT += qml dbus quick

SOURCES += main.cpp \
           yowsup.cpp \
		   yowsup_main.cpp \
		   yowsup_signals.cpp \
		   yowsup_methods.cpp \

HEADERS += yowsup.h \
		   yowsup_main.h \
		   yowsup_signals.h \
		   yowsup_methods.h

RESOURCES += whosthere.qrc
