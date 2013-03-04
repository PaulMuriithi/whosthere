/*
 * Copyright (C) 2013 Matthias Gehre <gehre.matthias@gmail.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QDebug>
#include "yowsup_main.h"
#include "whosthere.h"

using namespace std;

WhosThere::WhosThere(QQuickItem *parent) :
    QQuickItem(parent), ys(0), ym(0)
{
}

WhosThere::~WhosThere() {
    delete ys;
    delete ym;
}

void WhosThere::ready() {
    QDBusPendingReply<> r = ym->ready();
    r.waitForFinished();
    if( r.isError() ) {
        emit disconnected(QString("Dbus error on ready:") + r.error().message());
        return;
    }
}

void WhosThere::connectDBus() {
    if(ym && ys) {
        emit dbus_connected();
        return;
    }

    yowsup_main* yb = new yowsup_main("com.yowsup.methods", "/com/yowsup/methods", QDBusConnection::sessionBus());
    //This will show invalid, even though dbus activation will kick in at init() and launch yowsup
    /*if( !yb->isValid() ) {
        emit auth_fail(username, QString("Dbus error:") + yb->lastError().message());
        return;
    }*/
    yb->init("whosthere").waitForFinished();
    delete yb;

    ym = new yowsup_methods("com.yowsup.methods", "/com/yowsup/whosthere/methods", QDBusConnection::sessionBus());
    if( !ym->isValid() ) {
        emit dbus_fail(QString("Dbus error:") + ym->lastError().message());
        delete ym;
        ym = nullptr;
        return;
    }

    ym->disconnect("dbus_setup");

    /* connect com.yowsup.signal */
    ys = new yowsup_signals("com.yowsup.signals", "/com/yowsup/whosthere/signals", QDBusConnection::sessionBus());
    if( !ys->isValid() ) {
        emit dbus_fail(QString("Dbus error:") + ys->lastError().message());
        delete ym;
        delete ys;
        ym = nullptr;
        ys = nullptr;
        return;
    }

#define CN(X) connect(ys, &yowsup_signals::X, this, &WhosThere::X)
    CN(audio_received);
    CN(auth_fail);
    CN(auth_success);
    CN(contact_gotProfilePicture);
    CN(contact_gotProfilePictureId);
    CN(contact_paused);
    CN(contact_typing);
    CN(disconnected);
    CN(group_addParticipantsSuccess);
    CN(group_audioReceived);
    CN(group_createFail);
    CN(group_createSuccess);
    CN(group_endSuccess);
    CN(group_gotInfo);
    CN(group_gotParticipants);
    CN(group_gotPicture);
    CN(group_imageReceived);
    CN(group_infoError);
    CN(group_locationReceived);
    CN(group_messageReceived);
    CN(group_removeParticipantsSuccess);
    CN(group_setPictureError);
    CN(group_setPictureSuccess);
    CN(group_setSubjectSuccess);
    CN(group_subjectReceived);
    CN(group_vcardReceived);
    CN(group_videoReceived);
    CN(image_received);
    CN(location_received);
    CN(message_error);
    CN(message_received);
    CN(notification_contactProfilePictureUpdated);
    CN(notification_groupParticipantAdded);
    CN(notification_groupParticipantRemoved);
    CN(notification_groupPictureUpdated);
    CN(ping);
    //CN(pong);
    CN(presence_available);
    CN(presence_unavailable);
    CN(presence_updated);
    CN(profile_setPictureError);
    CN(profile_setPictureSuccess);
    CN(profile_setStatusSuccess);
    CN(receipt_messageDelivered);
    CN(receipt_messageSent);
    CN(receipt_visible);
    CN(status_dirty);
    CN(vcard_received);
    CN(video_received);
    CN(code_register_response);
    CN(code_request_response);
#undef CN
    qDebug() << "emit dbus_connected";
    emit dbus_connected();
}
/*
template<typename R, typename... T, typename... T2>
void WhosThere::callDbusMethod(QDBusPendingReply<R> (yowsup_methods::*f)(T...), T2&&... parameter) {
    auto lambda = [](R r) {;};
    callDbusMethod(f, lambda, std::forward<T2>(parameter)...);
}*/

/* Calls the given method on yowsup_methods with the given arguments in a new thread.
 * We have to duplicate the code below, because C++ does not allow template parameters to be void*/
template<typename... T, typename... T2>
void WhosThere::callDbusMethod(QDBusPendingReply<> (yowsup_methods::*f)(T...), T2&&... parameter) {
    QDBusPendingReply<> r = (ym->*f)(std::forward<T2>(parameter)...);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(r);
    auto lambda = [this] (QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<> r = *watcher;
            if( r.isError() )
                emit dbus_fail(r.error().message());
            watcher->deleteLater();
        };
    connect(watcher, &QDBusPendingCallWatcher::finished, lambda);
}

/* Calls the given method on yowsup_methods with the given arguments in a new thread */
template<typename R, typename... T, typename... T2>
void WhosThere::callDbusMethod_Callback(QDBusPendingReply<R> (yowsup_methods::*f)(T...),
                               std::function<void(const R&)> callback, T2&&... parameter) {

    QDBusPendingReply<> r = (ym->*f)(std::forward<T2>(parameter)...);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(r);
    auto lambda = [this, callback] (QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<R> r = *watcher;
            if( r.isError() )
                emit dbus_fail(r.error().message());
            else
                callback( r.value() );
            watcher->deleteLater();
        };
    connect(watcher, &QDBusPendingCallWatcher::finished, lambda);
}

void WhosThere::login(const QString& username, const QByteArray& password) {
    qDebug() << "WhosThere::login";
    callDbusMethod(&yowsup_methods::auth_login, username, QByteArray::fromBase64(password));
}

void WhosThere::code_request(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &identity, bool useText) {
    callDbusMethod(&yowsup_methods::code_request, countryCode, phoneNumber, identity, useText);
}

void WhosThere::code_register(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &code, const QByteArray &identity) {
    callDbusMethod(&yowsup_methods::code_register, countryCode, phoneNumber, code, identity);
}

#define DBUS_METHOD1(X, T) \
void WhosThere::X(T arg) { \
   callDbusMethod(&yowsup_methods::X, arg); \
}

#define DBUS_METHOD2(X, T1, T2) \
void WhosThere::X(T1 arg1, T2 arg2) { \
    callDbusMethod(&yowsup_methods::X, arg1, arg2); \
}

#define DBUS_METHOD4(X, T1, T2) \
void WhosThere::X(T1 arg1, T2 arg2, T3 arg3, T4 arg4) { \
    callDbusMethod(&yowsup_methods::X, arg1, arg2, arg3, arg4); \
}

/* Calls the dbus method X and emits X_completed with the result.
 * Don't use references. When the callback is invoked, they can have different values!
 */
#define DBUS_METHOD2_RETURN(X, RetVal, T1, T2) \
void WhosThere::X(T1 arg1, T2 arg2) { \
    auto callback = [this, arg1, arg2] (const RetVal& ret) { \
            emit X ## _completed(arg1, arg2, ret); \
        }; \
    callDbusMethod_Callback(&yowsup_methods::X, std::function<void(const RetVal&)>(callback), arg1, arg2); \
}

DBUS_METHOD1(pong,const QString&)
DBUS_METHOD1(disconnect,const QByteArray&)

DBUS_METHOD2(message_ack,const QString&,const QString&)
DBUS_METHOD2(subject_ack,const QString&,const QString&)
DBUS_METHOD2(notification_ack,const QString&,const QString&)
DBUS_METHOD2(visible_ack,const QString&,const QString&)
DBUS_METHOD2(delivered_ack,const QString&,const QString&)

/* Don't use references. When the callback is invoked, they can have different values! */
DBUS_METHOD2_RETURN(message_send, QString, QString, QByteArray)
