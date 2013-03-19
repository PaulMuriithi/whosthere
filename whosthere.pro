QT += qml dbus quick contacts
INCLUDEPATH += /usr/include/telepathy-qt5
LIBS += -ltelepathy-qt5

DEFINES += QMLJSDEBUGGER
QMAKE_CXXFLAGS += -Wall -std=c++0x

SOURCES += main.cpp \
           whosthere.cpp

HEADERS += whosthere.h

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
desktop.files = data/whosthere.desktop
INSTALLS += desktop

HEADERS += \
    imageprovider.h

SOURCES += \
    imageprovider.cpp

OTHER_FILES += \
    util.js
