QT += qml dbus quick

DEFINES += QMLJSDEBUGGER
QMAKE_CXXFLAGS += -Wall -std=c++0x

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

OTHER_FILES += \
    whosthere.qml

OTHER_FILES += \
    Conversation.qml

OTHER_FILES += \
    whosthere.js

target.path = $${PREFIX}/bin
INSTALLS += target
