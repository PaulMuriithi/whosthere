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
#ifndef YOWSUP_H
#define YOWSUP_H

#include <functional>
#include <QQuickItem>
#include <TelepathyQt/Types>
#include <TelepathyQt/Account>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/SimpleTextObserver>

using namespace Tp;
class TelepathyClient;

class WhosThere : public QQuickItem
{
    Q_OBJECT
public:
    explicit WhosThere(QQuickItem *parent = 0);
    ~WhosThere();

    /*
     * \param cc is the country code
     * \param phoneNumber (without country code)
     * \param uid some 32 byte lowercase hex
     * \param useText true for text, false for call
     * \param callback: will be called with status = 'sent', reason
     */
    static void requestCode(const QString& cc, const QString& phoneNumber,
                            const QString& uid, bool useText, std::function<void(const QString &, const QString&)> callback);
    /*
     * \param cc is the country code
     * \param phoneNumber (without country code)
     * \param uid some 32 byte lowercase hex
     * \param code Code you got via text, without hyphen (= 6 digits)
     * \param callback: will be called with status = 'ok', pw = password in base64
     */
    static void registerCode(const QString& cc, const QString& phoneNumber,
                             const QString& uid, const QString& code, std::function<void(const QString&,const QString&)> callback);
private:
    AccountManagerPtr mAM;
    ConnectionManagerPtr cm;
    AccountPtr mAccount;
    ConnectionPtr mConn;
    ClientRegistrarPtr mCR;
    SharedPtr<TelepathyClient> mHandler;
    SimpleTextObserverPtr m_simpleTextObserver;

    /* Telepathy */
    void onContactManagerStateChanged(ContactListState state);
    void onAMReady(PendingOperation *op);
    void onAccountFinished(PendingOperation* acc);
    void onAccountConnectionChanged(const ConnectionPtr &conn);
    void onPendingOperation(PendingOperation* op);
    void onAccountCreateFinished(PendingOperation* op);
    void onMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &channel);
    void onConnectionStatusChanged(uint status);
    void onAccountInvalidated();

public slots:
    void connectAccount();

    /* QML */
    void enableAccount(bool enabled);
    void removeAccount();
    void set_account(const QString& phonenumber, const QString &password);
    void disconnect();
    void code_request(const QString& cc, const QString& phonenumber, const QString& uid, bool useText);
    void code_register(const QString& cc, const QString& phonenumber, const QString& uid, const QString& code);
    void message_send(QString jid, QByteArray message);

signals:
    void noAccount();
    void accountOk();
    void connectionStatusChanged(QString status);
    /* Emitted when the account is enabled/disabled */
    void accountEnabledChanged(bool enabled);
    void accountValidityChanged(bool valid);
    void accountParametersChanged(QVariantMap parameters);
    void code_request_response(const QString &status, const QString &reason);
    void code_register_response(const QString &status, const QString &pw);


    void dbus_fail(const QString& reason);
    void dbus_connected();
    void message_send_completed(const QString &jid, const QString &message, const QString& msgId);
    void audio_received(const QString &msgId, const QString &jid, const QString &url, int size, bool wantsReceipt);
    void auth_fail(const QString &username, const QString &reason);
    void auth_success(const QString &username);

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
    void image_received(const QString &msgId, const QString &jid, const QString &preview, const QString &url, int size, bool wantsReceipt);
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
