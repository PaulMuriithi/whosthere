#include <QDebug>
#include "yowsup_main.h"
#include "yowsup.h"

YowSup::YowSup(QQuickItem *parent) :
    QQuickItem(parent), ys(0), ym(0)
{
}

void YowSup::ready() {
    QDBusPendingReply<> r = ym->ready();
    r.waitForFinished();
    if( r.isError() ) {
        emit disconnected(QString("Dbus error on ready:") + r.error().message());
        return;
    }
}

void YowSup::login(const QString& username, const QString& password) {

    qDebug() << "YowSup login";
    if(ym || ys) {
        emit auth_success(username);
        return;
    }

    yowsup_main* yb = new yowsup_main("com.yowsup.methods", "/com/yowsup/methods", QDBusConnection::sessionBus(), parent());
    //This will show invalid, even though dbus activation will kick in at init() and launch yowsup
    /*if( !yb->isValid() ) {
        emit auth_fail(username, QString("Dbus error:") + yb->lastError().message());
        return;
    }*/
    yb->init("whosthere").waitForFinished();
    delete yb;

    ym = new yowsup_methods("com.yowsup.methods", "/com/yowsup/whosthere/methods", QDBusConnection::sessionBus(), parent());
    if( !ym->isValid() ) {
        emit auth_fail(username, QString("Dbus error:") + ym->lastError().message());
        return;
    }

    ym->disconnect("");

    /* connect com.yowsup.signal */
    ys = new yowsup_signals("com.yowsup.signals", "/com/yowsup/whosthere/signals", QDBusConnection::sessionBus(), parent());
    if( !ys->isValid() ) {
        emit auth_fail(username, QString("Dbus error:") + ys->lastError().message());
        return;
    }

#define CN(X) connect(ys, &yowsup_signals::X, this, &YowSup::X)
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
#undef CN

    QDBusPendingReply<> r = ym->auth_login(username, password);
    r.waitForFinished();
    if( r.isError() ) {
        emit auth_fail(username, QString("Dbus error on auth_login:") + r.error().message());
        return;
    }
}

#define DBUS_METHOD1(X, T) \
void YowSup::X(T arg) { \
    if(!ym) {\
        qDebug() << "ym == NULL"; \
        return; \
    } \
    QDBusPendingReply<> r = ym->X(arg); \
    r.waitForFinished(); \
    if( r.isError() ) { \
        emit disconnected(QString("Dbus error on " #X ": ") + r.error().message()); \
        return; \
    } \
}

#define DBUS_METHOD2(X, T1, T2) \
void YowSup::X(T1 arg1, T2 arg2) { \
    if(!ym) {\
        qDebug() << "ym == NULL"; \
        return; \
    } \
    QDBusPendingReply<> r = ym->X(arg1, arg2); \
    r.waitForFinished(); \
    if( r.isError() ) { \
        emit disconnected(QString("Dbus error on " #X ": ") + r.error().message()); \
        return; \
    } \
}

#define DBUS_METHOD2_RETURN(X, R, T1, T2) \
R YowSup::X(T1 arg1, T2 arg2) { \
    if(!ym) {\
        qDebug() << "ym == NULL"; \
        return R(); \
    } \
    QDBusPendingReply<R> r = ym->X(arg1, arg2); \
    r.waitForFinished(); \
    if( r.isError() ) { \
        emit disconnected(QString("Dbus error on " #X ": ") + r.error().message()); \
        return R(); \
    } \
    return r.value(); \
}

DBUS_METHOD1(pong,const QString&)
DBUS_METHOD1(disconnect,const QByteArray&)

DBUS_METHOD2_RETURN(message_send,QString,const QString&,const QByteArray&)

DBUS_METHOD2(message_ack,const QString&,const QString&)
DBUS_METHOD2(subject_ack,const QString&,const QString&)
DBUS_METHOD2(notification_ack,const QString&,const QString&)
DBUS_METHOD2(visible_ack,const QString&,const QString&)
DBUS_METHOD2(delivered_ack,const QString&,const QString&)