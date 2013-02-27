#!/bin/sh
qdbus com.yowsup.methods /com/yowsup/methods org.freedesktop.DBus.Introspectable.Introspect > com.yowsup.methods.xml
qdbus com.yowsup.methods /com/yowsup/methods com.yowsup.methods.init 0
qdbus com.yowsup.methods /com/yowsup/0/methods org.freedesktop.DBus.Introspectable.Introspect > com.yowsup.0.methods.xml
qdbus com.yowsup.methods /com/yowsup/0/signals org.freedesktop.DBus.Introspectable.Introspect > com.yowsup.0.signals.xml

#qdbusxml2cpp -N -c yowsup \
#	         -p yowsup_dbus.h:yowsup_dbus.cpp \
#              com.yowsup.methods.xml \
#              com.yowsup.methods

qdbusxml2cpp -N -c yowsup_main \
             -p yowsup_main.h:yowsup_main.cpp \
              com.yowsup.methods.xml \
              com.yowsup.methods

qdbusxml2cpp -N -c yowsup_signals \
				-p yowsup_signals.h:yowsup_signals.cpp \
				com.yowsup.0.signals.xml \
				com.yowsup.signals

qdbusxml2cpp -N -c yowsup_methods \
				-p yowsup_methods.h:yowsup_methods.cpp \
				com.yowsup.0.methods.xml \
				com.yowsup.methods
