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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <TelepathyQt/Debug>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/PendingStringList>
#include <TelepathyQt/PendingSendMessage>
#include <TelepathyQt/ContactMessenger>
#include <TelepathyQt/TextChannel>

#include "whosthere.h"
#include "telepathyclient.h"

// #define USE_CLIENT

using namespace std;

/* --------------------------------- Account Manager ---------------------------------*/
WhosThere::WhosThere(QQuickItem *parent) :
    QQuickItem(parent)
{

    AccountFactoryPtr accountFactory = AccountFactory::create(QDBusConnection::sessionBus(),
        Account::FeatureCore);

    ConnectionFactoryPtr connectionFactory = ConnectionFactory::create(QDBusConnection::sessionBus(),
                Connection::FeatureConnected | Connection::FeatureRoster | Connection::FeatureSimplePresence);

    ChannelFactoryPtr channelFactory = ChannelFactory::create(QDBusConnection::sessionBus());

    ContactFactoryPtr contactFactory = ContactFactory::create(Contact::FeatureSimplePresence);

#ifdef USE_CLIENT
    /* TelepathyClient */
    mCR = ClientRegistrar::create(accountFactory, connectionFactory, channelFactory);
    mHandler = TelepathyClient::create();
    QString handlerName(QLatin1String("WhosThereGui"));
    if (!mCR->registerClient(AbstractClientPtr::dynamicCast(mHandler), handlerName)) {
        qWarning() << "Unable to register incoming file transfer handler, aborting";
    }
    connect(mHandler.data(), &TelepathyClient::messageReceived,
            this, &WhosThere::onMessageReceived);
    connect(mHandler.data(), &TelepathyClient::messageSent,
            this, &WhosThere::onMessageSent);
#endif

    /* AccountManager */
    mAM = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);
    qDebug() << "Waiting for account manager";
    connect(mAM->becomeReady(), &PendingOperation::finished,
            this, &WhosThere::onAMReady);
    /*connect(mAM.data(),
                SIGNAL(newAccount(const Tp::AccountPtr &)),
                SLOT(onNewAccount(const Tp::AccountPtr &)));*/
}


WhosThere::~WhosThere() {

}

void WhosThere::onAMReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "AM cannot become ready -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }

    QList<AccountPtr> accounts = mAM->accountsByProtocol("whatsapp")->accounts();
    qDebug() << "number of accounts: " << accounts.size();
    if(accounts.size() == 0) {
        qDebug() << "Creating new account";
        emit noAccount();
    } else {

        emit accountOk();
        mAccount = *accounts.begin();
        connect(mAccount->becomeReady(), &PendingOperation::finished,
                this, &WhosThere::onAccountFinished);
    }
}

/* --------------------------------- Account ---------------------------------*/

void WhosThere::set_account(const QString& phonenumber, const QString& password)
{
    QVariantMap parameters;
    if(phonenumber.length() > 0)
        parameters.insert( "account", QVariant(phonenumber));
    if(password.length() > 0)
        parameters.insert( "password", QVariant(password));

    if(mAccount.isNull()) {
        if(phonenumber.length() == 0 || password.length() == 0) {
            qWarning() << "WhosThere::set_account: phonenumber.length() == 0 || password.length() == 0";
            return;
        }
        QVariantMap properties;
        properties.insert( "org.freedesktop.Telepathy.Account.Enabled", true );
        PendingAccount* acc = mAM->createAccount("whosthere", "whatsapp", "WhatApp Account", parameters, properties);

        connect(acc, &PendingAccount::finished,
                this, &WhosThere::onAccountCreateFinished );
    } else {
        PendingStringList* sl = mAccount->updateParameters(parameters, QStringList());
        connect(sl, &Tp::PendingStringList::finished,
                this, &WhosThere::onPendingOperation);
    }
}

void WhosThere::connectAccount() {
    if(!mAccount.isNull())
        mAccount->setRequestedPresence(Presence::available());
}

void WhosThere::removeAccount() {
    if(!mAccount.isNull())
        mAccount->remove();
}

void WhosThere::disconnect() {
    if(!mAccount.isNull())
        mAccount->setRequestedPresence(Presence::offline());
}

void WhosThere::onAccountCreateFinished(PendingOperation* op) {
    if (op->isError()) {
        qWarning() << "Account cannot become created -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }
    emit accountOk();
    mAccount = dynamic_cast<PendingAccount*>(op)->account();
    connect(mAccount->becomeReady(), &PendingOperation::finished,
            this, &WhosThere::onAccountFinished);
}

void WhosThere::onAccountFinished(PendingOperation* op) {
    if (op->isError()) {
        qWarning() << "Account cannot become ready -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }
    if(mAccount.isNull()) {
        qDebug() << "hosThere::onAccountFinished: mAccount == NULL";
        return;
    }
    //qDebug() << "WhosThere::setAccount nickname: " << account->cmName();
    //qDebug() << "WhosThere::setAccount valid : " << mAccount->isValidAccount() << " enabled: " << mAccount->isEnabled();
    connect( mAccount.data(), &Account::stateChanged,
             this, &WhosThere::accountEnabledChanged);
    accountEnabledChanged(mAccount->isEnabled());

    connect( mAccount.data(), &Account::validityChanged,
             this, &WhosThere::accountValidityChanged);
    accountValidityChanged(mAccount->isValid());

    connect( mAccount.data(), &Account::invalidated,
             this, &WhosThere::onAccountInvalidated);

    connect( mAccount.data(), &Account::parametersChanged,
             this, &WhosThere::accountParametersChanged);
    accountParametersChanged( mAccount->parameters() );

#ifndef USE_CLIENT
    m_simpleTextObserver = SimpleTextObserver::create(mAccount);
    connect(m_simpleTextObserver.data(), &SimpleTextObserver::messageReceived,
            this, &WhosThere::onMessageReceived);
    connect(m_simpleTextObserver.data(), &SimpleTextObserver::messageSent,
            this, &WhosThere::onMessageSent);
#endif

    connect(mAccount.data(), &Account::connectionChanged,
            this, &WhosThere::onAccountConnectionChanged);
    onAccountConnectionChanged(mAccount->connection());
}

void WhosThere::enableAccount(bool enabled) {
    if(!mAccount.isNull())
        connect(mAccount->setEnabled(enabled), &PendingOperation::finished,
                this, &WhosThere::onPendingOperation);
}

void WhosThere::onAccountInvalidated() {
    mAccount.reset();
    emit noAccount();
}

void WhosThere::onPendingOperation(PendingOperation* acc) {
    if (acc->isError()) {
        qWarning() << "Pending operation failed: " <<
            acc->errorName() << ": " << acc->errorMessage();
        return;
    }
}

/* --------------------------------- Connection ---------------------------------*/
void WhosThere::onAccountConnectionChanged(const ConnectionPtr &conn)
{
    if (conn) {
        qDebug() << "WhosThere::onAccountConnectionChanged";
        mConn = conn;
        connect(mConn.data(), &Connection::statusChanged,
                this, &WhosThere::onConnectionStatusChanged);
        onConnectionStatusChanged( mConn->status() );
        connect(mConn->contactManager().data(), &ContactManager::stateChanged,
                this, &WhosThere::onContactManagerStateChanged);
        onContactManagerStateChanged(mConn->contactManager()->state());
    } else {
        emit connectionStatusChanged("disconnected");
        qDebug() << " WhosThere::onAccountConnectionChanged: conn = NULL";
    }
}

void WhosThere::onConnectionStatusChanged(uint status) {
    switch(status) {
    case ConnectionStatusDisconnected:
        emit connectionStatusChanged("disconnected");
        connectAccount();
        break;
    case ConnectionStatusConnecting:
        emit connectionStatusChanged("connecting");
        break;
    case ConnectionStatusConnected:
        emit connectionStatusChanged("connected");
        break;
    }
}

/* --------------------------------- ContactManger ---------------------------------*/
void WhosThere::onContactManagerStateChanged(ContactListState state)
{
    qDebug() << "WhosThere::onContactManagerStateChanged " << state;
    if (state == ContactListStateSuccess) {
        connect(mConn->contactManager().data(), &ContactManager::allKnownContactsChanged,
                this, &WhosThere::onContactsChanged);
        qDebug() << "Loading contacts";
        onNewContacts(mConn->contactManager()->allKnownContacts());
    }
}

void WhosThere::onContactsChanged(const Tp::Contacts &  	contactsAdded,
                                  const Tp::Contacts &  	contactsRemoved,
                                  const Tp::Channel::GroupMemberChangeDetails &  	details) {

    qDebug() << "WhosThere::onContactsChanged";
    onNewContacts(contactsAdded);
}

void WhosThere::onNewContacts(const Tp::Contacts& contacts) {

    foreach (const ContactPtr &contact, contacts) {
        QString jid = contact->id();
        emit newContact(jid);
        emit presenceChanged(jid, contact->presence().status());

        connect(contact.data(), &Contact::presenceChanged,
                [this,jid](const Tp::Presence& presence) {
                        emit presenceChanged(jid, presence.status());
                });
    }
}

/* --------------------------------- SimpleTextObserver ---------------------------------*/
void WhosThere::onMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &channel) {
    qDebug() << "WhosThere::onMessageReceived isDeliveryReport: " << message.isDeliveryReport()
             << " text: " << message.text()
             << " sender: " << message.sender()->id();

    if(message.isDeliveryReport()) {
        ReceivedMessage::DeliveryDetails details = message.deliveryDetails();
        if(! details.hasOriginalToken() ) {
            qWarning() << "WhosThere::onMessageReceived: does not have original token";
            return;
        }
        QString jid = message.sender()->id();
        QString msgId = details.originalToken();
        if(details.status() == DeliveryStatusAccepted)
            emit messageSent(jid, msgId);
        else if(details.status() == DeliveryStatusDelivered )
            emit messageDelivered(jid, msgId);
        else
            qWarning() << "WhosThere::onMessageReceived: cannot handle delivery status " << details.status();

    } else {

        QVariantMap data;

        data["type"] = "message";
        data["content"] = message.text();
        data["jid"] = message.sender()->id();
        if(message.header().contains("message-token"))
            data["msgId"] = message.header()["message-token"].variant();
        else {
            qWarning() << "message does not have message-token field!";
            return;
        }
        if(message.header().contains("message-received"))
            data["timestamp"] = message.header()["message-received"].variant();
        data["incoming"] = 1;

        emit newMessage(data);
    }
}

void WhosThere::onMessageSent ( const Tp::Message& message,
                                Tp::MessageSendingFlags /*flags*/,
                                const QString& msgId,
                                const Tp::TextChannelPtr& channel) {

    onMessageSent2(message, msgId, channel->targetId());
}

void WhosThere::onMessageSent2 ( const Tp::Message& message,
                                const QString& msgId,
                                const QString& jid) {

    QVariantMap data;
    data["type"] = "message";
    data["content"] = message.text();
    data["jid"] = jid;
    data["msgId"] = msgId;
    if(message.header().contains("message-sent"))
        data["timestamp"] = message.header()["message-sent"].variant();
    else {
        qWarning() << "No timestamp in sent message, fix CM!";
        data["timestamp"] = QDateTime::currentMSecsSinceEpoch()/1000;
    }
    data["incoming"] = 0;

    emit newMessage(data);
}

void WhosThere::message_send(QString jid, QByteArray message)
{
#if 0
    connect(mAccount->ensureAndHandleTextChat(jid), &PendingChannel::finished,
            [this,message](PendingOperation* op) {
        if (op->isError()) {
            qWarning() << "ensureChannel failed: " <<
                op->errorName() << ": " << op->errorMessage();
            return;
        }
        TextChannelPtr channel = TextChannelPtr::dynamicCast((dynamic_cast<PendingChannel*>(op))->channel());
        MessagePartList partList;
        MessagePart header, body;
        header["message-type"]          = QDBusVariant(ChannelTextMessageTypeNormal);
        body["content-type"]            = QDBusVariant("text/plain");
        body["content"]                 = QDBusVariant(message);
        partList << header << body;
        connect(channel->send(partList), &PendingSendMessage::finished,
                [](PendingOperation* op) {
                    if (op->isError()) {
                        qWarning() << "channel->send failed: " <<
                        op->errorName() << ": " << op->errorMessage();
                        return;
                    }
                    op->deleteLater();
                });
        //op->deleteLater();
    });

#else
    //This does not work on newly created channels
    static ContactMessengerPtr contactMessenger;

    if(contactMessenger.isNull() || contactMessenger->contactIdentifier() != jid) {
        qDebug() << "WhosThere::message_send: creating new ContactMessenger";
        contactMessenger = ContactMessenger::create(mAccount,jid);
        if(contactMessenger.isNull()) {
            qWarning() << "WhosThere::message_send: contactMessenger = NULL";
            return;
        }
    }
#if 1
    contactMessenger->sendMessage(message);
#else
    connect(contactMessenger->sendMessage(message), &PendingOperation::finished,
            [this, jid](PendingOperation* op) {
                if (op->isError()) {
                    qWarning() << "ontactMessenger->sendMessage() failed: " <<
                    op->errorName() << ": " << op->errorMessage();
                    return;
                }
                PendingSendMessage* sendMessage = dynamic_cast<PendingSendMessage*>(op);
                onMessageSent2(sendMessage->message(), sendMessage->sentMessageToken(), jid);
                op->deleteLater();
            });
#endif
#endif
}

/* --------------------------------- Registration ---------------------------------*/
void WhosThere::code_request(const QString& cc, const QString& phonenumber, const QString &uid, bool useText)
{
    if(uid.length() != 32){
        qWarning() << "WhosThere::code_request : uid.length() != 32";
        return;
    }
    WhosThere::requestCode(cc, phonenumber, uid, useText,
                           [this] (const QString& status, const QString& reason) {
                                emit code_request_response(status, reason);
                            });
}

void WhosThere::code_register(const QString& cc, const QString& phonenumber, const QString& uid, const QString& code_)
{
    if(uid.length() != 32) {
        qWarning() << "WhosThere::code_register : uid.length() != 32";
        return;
    }
    QString code = code_;
    if(code.length() == 7) //remove hyphon
        code = code.left(3) + code.right(3);
    if(code.length() != 6) {
        qWarning() << "WhosThere::code_register : code.length() != 6";
        return;
    }

    WhosThere::registerCode(cc, phonenumber, uid, code,
                           [this] (const QString& status, const QString& pw) {
                                emit code_register_response(status, pw);
                            });
}

QString makeToken(const QString& phoneNumber) {
    /*return QCryptographicHash::hash(
    (QLatin1String("PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk1354754753509") + phoneNumber).toLatin1(),
                         QCryptographicHash::Md5).toHex();*/
    return QCryptographicHash::hash(
        (QLatin1String("PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk1359594496554") + phoneNumber).toLatin1(),
                             QCryptographicHash::Md5).toHex();
}

//"WhatsApp/2.3.53 S40Version/14.26 Device/Nokia302"
const QLatin1String UserAgent("WhatsApp/2.4.7 S40Version/14.26 Device/Nokia302");

void WhosThere::requestCode(const QString& cc, const QString& phoneNumber,
                            const QString& uid, bool useText, std::function<void(const QString&, const QString&)> callback) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("cc", cc);
    urlQuery.addQueryItem("in", phoneNumber);
    urlQuery.addQueryItem("lc", "US");
    urlQuery.addQueryItem("lg", "en");
    urlQuery.addQueryItem("mcc", "000");
    urlQuery.addQueryItem("mnc", "000");
    urlQuery.addQueryItem("method", useText ? "sms" : "voice");
    urlQuery.addQueryItem("id", uid);
    urlQuery.addQueryItem("token", makeToken(phoneNumber));

    QUrl url("https://v.whatsapp.net/v2/code");
    url.setQuery(urlQuery);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, UserAgent);
    request.setRawHeader("Accept","text/json");
    request.setUrl(url);
    qDebug() << url;

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished,
            [manager,callback] (QNetworkReply* reply) {
                    if(reply->error() != QNetworkReply::NoError) {
                        qDebug() << "Http error " << reply->error();
                        callback("fail", "http error");
                    } else {
                        QByteArray data = reply->readAll();
                        qDebug() << "Reply: " << data;
                        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
                        QJsonObject jsonObject = jsonDocument.object();
                        if(!jsonObject.contains("status")) {
                            qDebug() << "status: " << jsonObject["status"];
                            callback("fail","malformed");
                        } else {
                            QString reason;
                            if(jsonObject.contains("reason"))
                                reason = jsonObject["status"].toString();
                            callback(jsonObject["status"].toString(), reason);
                        }
                    }
                    reply->deleteLater();
                    manager->deleteLater();
                });
    manager->get(request);
}

void WhosThere::registerCode(const QString& cc, const QString& phoneNumber,
                        const QString& uid, const QString& code, std::function<void(const QString&,const QString&)> callback) {
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("cc", cc);
    urlQuery.addQueryItem("in", phoneNumber);
    urlQuery.addQueryItem("id", uid);
    urlQuery.addQueryItem("code", code);
    urlQuery.addQueryItem("token", makeToken(phoneNumber));

    QUrl url("https://v.whatsapp.net/v2/register");
    url.setQuery(urlQuery);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, UserAgent);
    request.setRawHeader("Accept","text/json");
    request.setUrl(url);
    qDebug() << url;

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished,
            [manager,callback] (QNetworkReply* reply) {
                    if(reply->error() != QNetworkReply::NoError) {
                        qDebug() << "Http error " << reply->error();
                        callback("http error","");
                    } else {
                        QByteArray data = reply->readAll();
                        qDebug() << "Reply: " << data;
                        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
                        QJsonObject jsonObject = jsonDocument.object();
                        if(!jsonObject.contains("status") || !jsonObject.contains("pw")) {
                            qDebug() << "status: " << jsonObject["status"];
                            callback("malformed","");
                        } else {
                            callback(jsonObject["status"].toString(),jsonObject["pw"].toString());
                        }
                    }
                    reply->deleteLater();
                    manager->deleteLater();
                });
    manager->get(request);
}
