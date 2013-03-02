QT += qml dbus quick

DEFINES += QMLJSDEBUGGER
QMAKE_CXXFLAGS += -Wall -std=c++0x

SOURCES += main.cpp \
           whosthere.cpp \
		   yowsup_main.cpp \
		   yowsup_signals.cpp \
		   yowsup_methods.cpp \

HEADERS += whosthere.h \
		   yowsup_main.h \
		   yowsup_signals.h \
		   yowsup_methods.h

RESOURCES += whosthere.qrc

OTHER_FILES += \
    whosthere.qml

OTHER_FILES += \
    Conversation.qml

OTHER_FILES += \
    db.js

target.path = $${PREFIX}/bin
INSTALLS += target

desktop.path = $${PREFIX}/share/applications/
desktop.files = whosthere.desktop
INSTALLS += desktop

HEADERS += \
    imageprovider.h

SOURCES += \
    imageprovider.cpp
