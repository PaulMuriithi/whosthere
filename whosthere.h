#ifndef YOWSUP_H
#define YOWSUP_H

#include <QQuickItem>
#include "yowsup_signals.h"
#include "yowsup_methods.h"

class WhosThere : public QQuickItem
{
    Q_OBJECT
public:
    explicit WhosThere(QQuickItem *parent = 0);
    yowsup_signals* ys;
    yowsup_methods* ym;

    bool connectDbus();
    static WhosThere* get();
public slots:

    /* Misc */
    void login(const QString &username, const QString &password);
    void ready();
    void pong(const QString &pingId);
    void disconnect(const QByteArray& reason);

    void code_register(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &code, const QByteArray &identity);
    void code_request(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &identity, bool useText);
    //void ping();

    /*void typing_send(const QDBusVariant &jid);
    void typing_paused(const QDBusVariant &jid);
    void clientconfig_send();
    void picture_getIds(const QDBusVariant &jids);
    void getVersion();

    void contact_getProfilePicture(const QDBusVariant &jid);*/

    /* Profile */
    /*void profile_setStatus(const QDBusVariant &status);
    void profile_setPicture(const QDBusVariant &filepath);
    void profile_getPicture();*/

    /* Presence */
    /*void presence_unsubscribe(const QDBusVariant &jid);
    void presence_subscribe(const QDBusVariant &jid);
    void presence_sendUnavailable();
    void presence_sendAvailableForChat();
    void presence_sendAvailable();
    void presence_request(const QDBusVariant &jid);*/

    /* Message */
    //void message_videoSend(const QDBusVariant &jid, const QDBusVariant &url, const QDBusVariant &name, const QDBusVariant &size, const QDBusVariant &preview);
    //void message_vcardSend(const QDBusVariant &jid, const QDBusVariant &data, const QDBusVariant &name);
    QString message_send(const QString &jid, const QByteArray &message);
    //void message_locationSend(const QDBusVariant &jid, const QDBusVariant &latitude, const QDBusVariant &longitude, const QDBusVariant &preview);
    //void message_imageSend(const QString &jid, const QString &url, const QString &name, int size, const QString &preview);
    //void message_audioSend(const QDBusVariant &jid, const QDBusVariant &url, const QDBusVariant &name, const QDBusVariant &size);

    /* Group */
    /* todo */

    /* Ack */
    /* group_subjectReceived!, message_received */
    void message_ack(const QString& jid, const QString &messageId);
    /* ??? */
    void visible_ack(const QString &jid, const QString &msgId);
    /* ??? */
    void subject_ack(const QString &jid, const QString &msgId);
    /* For notification_contactProfilePictureUpdated!, notification_groupPictureUpdated!,
     * notification_groupParticipantAdded!, notification_groupParticipantRemoved! */
    void notification_ack(const QString &jid, const QString &msgId);
    /* For receipt_messageDelivered, profile_setStatusSuccess! */
    void delivered_ack(const QString &jid, const QString &msgId);

signals:
    QByteArray getPreviewImage(const QString& id);
    /* Keep in sync with yowsup_signals.h */
    void audio_received(const QString &msgId, const QString &jid, const QString &url, int size, bool wantsReceipt);
    void auth_fail(const QString &username, const QString &reason);
    void auth_success(const QString &username);
    void code_register_response(const QString &status, const QString &reason, const QString &pw);
    void code_request_response(const QString &status, const QString &reason);
    void contact_gotProfilePicture(const QString &jid, const QString &filename);
    void contact_gotProfilePictureId(const QString &jid, const QString &pictureId);
    void contact_paused(const QString &jid);
    void contact_typing(const QString &jid);
    void disconnected(const QString &reason);
    void group_addParticipantsSuccess(const QString &jid);
    void group_audioReceived(const QString &msgId, const QString &jid, const QString &author, const QString &url, int size, bool wantsReceipt);
    void group_createFail(const QString &errorCode);
    void group_createSuccess(const QString &jid, const QString &group_id);
    void group_endSuccess(const QString &jid);
    void group_gotInfo(const QString &jid, const QString &owner, const QString &subject, const QString &subjectOwner, const QString &subjectT, const QString &creation);
    void group_gotParticipants(const QString &jid, const QString &jids);
    void group_gotPicture(const QString &jid, const QString &filepath);
    void group_imageReceived(const QString &msgId, const QString &jid, const QString &author, const QString &preview, const QString &url, int size, bool wantsReceipt);
    void group_infoError(const QString &errorCode);
    void group_locationReceived(const QString &msgId, const QString &jid, const QString &author, const QString &name, const QString &preview, double latitude, double longitude, bool wantsReceipt);
    void group_messageReceived(const QString &msgId, const QString &jid, const QString &author, const QString &content, const QString &timestamp, bool wantsReceipt);
    void group_removeParticipantsSuccess(const QString &jid);
    void group_setPictureError(const QString &jid, const QString &errorCode);
    void group_setPictureSuccess(const QString &jid);
    void group_setSubjectSuccess(const QString &jid);
    void group_subjectReceived(const QString &msgId, const QString &fromAttribute, const QString &author, const QString &newSubject, const QString &timestamp, const QString &receiptRequested);
    void group_vcardReceived(const QString &msgId, const QString &jid, const QString &author, const QString &name, const QString &data, bool wantsReceipt);
    void group_videoReceived(const QString &msgId, const QString &jid, const QString &author, const QString &preview, const QString &url, int size, bool wantsReceipt);
    void image_received(const QString &msgId, const QString &jid, const QByteArray &preview, const QString &url, int size, bool wantsReceipt);
    void location_received(const QString &msgId, const QString &jid, const QString &name, const QString &preview, double latitude, double longitude, bool wantsReceipt);
    void message_error(const QString &msgId, const QString &jid, const QString &errorCode);
    void message_received(const QString &msgId, const QString &jid, const QString &content, int timestamp, bool wantsReceipt, const QString &pushName);
    void notification_contactProfilePictureUpdated(const QString &jid, const QString &timestamp, const QString &msgId, bool wantsReceipt);
    void notification_groupParticipantAdded(const QString &gJid, const QString &jid, const QString &author, const QString &timestamp, const QString &msgId, bool wantsReceipt);
    void notification_groupParticipantRemoved(const QString &gjid, const QString &jid, const QString &author, const QString &timestamp, const QString &msgId, bool wantsReceipt);
    void notification_groupPictureUpdated(const QString &jid, const QString &author, const QString &timestamp, const QString &msgId, bool wantsReceipt);
    void ping(const QString &pingId);
    void pong();
    void presence_available(const QString &jid);
    void presence_unavailable(const QString &jid);
    void presence_updated(const QString &jid, const QString &lastSeen);
    void profile_setPictureError(const QString &errorCode);
    void profile_setPictureSuccess();
    void profile_setStatusSuccess(const QString &jid, const QString &msgId);
    void receipt_messageDelivered(const QString &jid, const QString &msgId);
    void receipt_messageSent(const QString &jid, const QString &msgId);
    void receipt_visible(const QString &jid, const QString &msgId);
    void status_dirty();
    void vcard_received(const QString &msgId, const QString &jid, const QString &name, const QString &data, bool wantsReceipt);
    void video_received(const QString &msgId, const QString &jid, const QString &preview, const QString &url, int size, bool wantsReceipt);
};

#endif // YOWSUP_H
