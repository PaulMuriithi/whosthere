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
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/ContactMessenger>
#include <TelepathyQt/TextChannel>

#include <QContactPhoneNumber>
#include <QContactDisplayLabel>

#include "whosthere.h"

using namespace std;
using namespace QtContacts;

WhosThere::~WhosThere() {

}

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

    /* AccountManager */
    mAM = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);
    qDebug() << "Waiting for account manager";
    connect(mAM->becomeReady(), &PendingOperation::finished,
            this, &WhosThere::onAMReady);

    connect(&contactManager,&QtContacts::QContactManager::contactsAdded,
            this, &WhosThere::onQContactManagerContactsAdded);
}



void WhosThere::onAMReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "AM cannot become ready -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }

    connect(mAM.data(), &AccountManager::newAccount,
            this, &WhosThere::onNewAccount);

    QList<AccountPtr> accounts = mAM->accountsByProtocol("whatsapp")->accounts();
    qDebug() << "number of accounts: " << accounts.size();
    if(accounts.size() == 0) {
        emit noAccount();
    } else {

        emit accountOk();
        mAccount = *accounts.begin();
        connect(mAccount->becomeReady(), &PendingOperation::finished,
                this, &WhosThere::onAccountFinished);
    }
}

void WhosThere::onNewAccount(const Tp::AccountPtr &account) {
    qDebug() << "WhosThere::OnNewAccount " << account->normalizedName();
    if(mAccount.isNull()) {
        emit accountOk();
        mAccount = account;
        connect(mAccount->becomeReady(), &PendingOperation::finished,
                this, &WhosThere::onAccountFinished);
    } else {
        emit alert("Multiple acccounts detected. Use mc-tool to remove all but one!");
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

        connect(mAM->createAccount("whosthere", "whatsapp", "WhatApp Account", parameters, properties),
                &PendingAccount::finished,
                this, &WhosThere::onAccountCreateFinished );
    } else {
        PendingStringList* sl = mAccount->updateParameters(parameters, QStringList());
        connect(sl, &Tp::PendingStringList::finished,
                this, &WhosThere::onPendingOperation);
    }
}

void WhosThere::connectAccount() {
    if(!mAccount.isNull())
        connect(mAccount->setRequestedPresence(Presence::available()), &PendingOperation::finished,
                this, &WhosThere::onPendingOperation);
       }

void WhosThere::removeAccount() {
    if(!mAccount.isNull())
        connect(mAccount->remove(),&PendingOperation::finished,
                this, &WhosThere::onPendingOperation);
}

void WhosThere::disconnect() {
    if(!mAccount.isNull())
        connect(mAccount->setRequestedPresence(Presence::offline()), &PendingOperation::finished,
                this, &WhosThere::onPendingOperation);
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

    connect( mAccount.data(), &Account::connectsAutomaticallyPropertyChanged,
             this, &WhosThere::accountAlwaysConnectedChanged);
    accountAlwaysConnectedChanged(mAccount->connectsAutomatically());

    connect( mAccount.data(), &Account::validityChanged,
             this, &WhosThere::accountValidityChanged);
    accountValidityChanged(mAccount->isValid());

    connect( mAccount.data(), &Account::invalidated,
             this, &WhosThere::onAccountInvalidated);

    connect( mAccount.data(), &Account::parametersChanged,
             this, &WhosThere::accountParametersChanged);
    accountParametersChanged( mAccount->parameters() );

    connect( mAccount.data(), &Account::onlinenessChanged,
             this, &WhosThere::onOnlinenessChanged);
    onOnlinenessChanged( mAccount->isOnline() );

    m_simpleTextObserver = SimpleTextObserver::create(mAccount);
    connect(m_simpleTextObserver.data(), &SimpleTextObserver::messageReceived,
            this, &WhosThere::onMessageReceived);
    connect(m_simpleTextObserver.data(), &SimpleTextObserver::messageSent,
            this, &WhosThere::onMessageSent);

    connect(mAccount.data(), &Account::connectionChanged,
            this, &WhosThere::onAccountConnectionChanged);
    onAccountConnectionChanged(mAccount->connection());
}

void WhosThere::enableAccount(bool enabled) {
    if(!mAccount.isNull())
        connect(mAccount->setEnabled(enabled), &PendingOperation::finished,
                this, &WhosThere::onPendingOperation);
}

void WhosThere::alwaysConnected(bool enabled) {
     if(!mAccount.isNull())
         connect(mAccount->setConnectsAutomatically(enabled), &PendingOperation::finished,
                 this, &WhosThere::onPendingOperation);
}

void WhosThere::onAccountInvalidated() {
    qDebug() << "WhosThere::onAccountInvalidated";
    mAccount.reset();
    emit noAccount();
}

void WhosThere::onOnlinenessChanged(bool online) {
    qDebug() << "WhosThere::onOnlinenessChanged " << online;
    if(!mAccount->connection().isNull())
        onConnectionStatusChanged(mAccount->connection()->status());
    else if(online)
        qWarning() << "Got WhosThere::onOnlinenessChanged, but no connection";
}

void WhosThere::onPendingOperation(PendingOperation* op) {
    if (op->isError()) {
        QString msg;
        QTextStream(&msg) << "Pending operation failed: " <<
            op->errorName() << ": " << op->errorMessage();
        emit alert(msg);
    }
    qDebug() << "WhosThere::onPendingOperation";
    op->deleteLater();
}

/* --------------------------------- Connection ---------------------------------*/
void WhosThere::onAccountConnectionChanged(const ConnectionPtr &conn)
{
    if (conn) {
        qDebug() << "WhosThere::onAccountConnectionChanged";

        connect(mAccount->connection().data(), &Connection::statusChanged,
                this, &WhosThere::onConnectionStatusChanged);
        onConnectionStatusChanged( mAccount->connection()->status() );

        connect(mAccount->connection()->contactManager().data(), &ContactManager::stateChanged,
                this, &WhosThere::onContactManagerStateChanged);
        onContactManagerStateChanged( mAccount->connection()->contactManager()->state());
    } else {
        emit connectionStatusChanged("disconnected", "");
        qDebug() << " WhosThere::onAccountConnectionChanged: conn = NULL";
    }
}

void WhosThere::onConnectionStatusChanged(ConnectionStatus status) {
    QString reason;
    switch(mAccount->connection()->statusReason()) {
    case ConnectionStatusReasonNoneSpecified:
        reason = "none";
        break;
    case ConnectionStatusReasonRequested:
        reason = "requested";
        break;
    case ConnectionStatusReasonNetworkError:
        reason = "network error";
        break;
    case ConnectionStatusReasonAuthenticationFailed:
        reason = "authentication failed";
        break;
    default:
        QTextStream(&reason) << "unknown " << mAccount->connection()->statusReason();
    }

    switch(status) {
    case ConnectionStatusDisconnected:
        emit connectionStatusChanged("disconnected", reason);
        if(mAccount->connection()->statusReason() != ConnectionStatusReasonAuthenticationFailed
           && mAccount->connection()->statusReason() != ConnectionStatusReasonRequested)
            connectAccount();
        break;
    case ConnectionStatusConnecting:
        emit connectionStatusChanged("connecting", reason);
        break;
    case ConnectionStatusConnected:
        emit connectionStatusChanged("connected", reason);
        break;
    default:
        qWarning() << "WhosThere::onConnectionStatusChanged: Undefined connection status";
    }
}

/* --------------------------------- ContactManger ---------------------------------*/
void WhosThere::onContactManagerStateChanged(ContactListState state)
{
    qDebug() << "WhosThere::onContactManagerStateChanged " << state;
    if (state == ContactListStateSuccess) {
        connect(mAccount->connection()->contactManager().data(), &ContactManager::allKnownContactsChanged,
                this, &WhosThere::onContactsChanged);
        qDebug() << "Loading contacts";
        onNewContacts(mAccount->connection()->contactManager()->allKnownContacts());
    }
}

void WhosThere::onContactsChanged(const Tp::Contacts &  	contactsAdded,
                                  const Tp::Contacts &  	/*contactsRemoved*/,
                                  const Tp::Channel::GroupMemberChangeDetails &  	/*details*/) {

    qDebug() << "WhosThere::onContactsChanged";
    onNewContacts(contactsAdded);
}

void WhosThere::onNewContacts(const Tp::Contacts& contacts) {

    foreach (const ContactPtr &contact, contacts) {
        QString jid = contact->id();
        emit newContact(jid);
        connect(contact.data(), &Contact::presenceChanged,
                [this,jid](const Tp::Presence& presence) {
                        emit presenceChanged(jid, presence.status());
                });
        emit presenceChanged(jid, contact->presence().status());
    }
}

/* Addresbook syncing */
void WhosThere::syncAddressbook() {
    qDebug() << "WhosThere::syncAddressbook";
    QRegExp validNumberRegExp("\\+?\\d+");

    if(mAccount.isNull()
            || mAccount->connection().isNull()
            || mAccount->connection()->contactManager().isNull()) {
        alert("Not connected. Please connect for sync");
        return;
    }

    //List all available managers for debugging purpose
    QStringList managers = QtContacts::QContactManager::availableManagers();
    for( auto avmanager : managers)
        qDebug() << "Available managers: " << avmanager;

    qDebug() << "Current manager: " << contactManager.managerName();
    QList<QtContacts::QContact> contacts = contactManager.contacts();
    QStringList phoneNumbers;
    for(const QtContacts::QContact& contact : contacts)
    {
        //qDebug() << "contact: " << contact;
        QList<QtContacts::QContactPhoneNumber> contactPhoneNumbers = contact.details<QtContacts::QContactPhoneNumber>();
        for(const QtContacts::QContactPhoneNumber& detail : contactPhoneNumbers) {
            QString phoneNumber =  detail.number();

            sanitizePhonenumber(phoneNumber);

            if( !validNumberRegExp.exactMatch(phoneNumber)) {
                qDebug() << "Ignoring number " << phoneNumber;
                continue;
            }
            qDebug() << "Syncing number " << phoneNumber;
            phoneNumbers.append(phoneNumber);
        }
    }
    connect(mAccount->connection()->contactManager()->contactsForVCardAddresses("tel",phoneNumbers),
            &PendingOperation::finished,
            this, &WhosThere::onPendingOperation);
}

QString WhosThere::getNameForUID(const QString& uid) {
    if(uid.length() == 0)
            return QString();

    if(!uid.endsWith("@s.whatsapp.net")) {
        qDebug() << "WhosThere::getNameForUID: invalid uid " << uid;
        return QString();
    }
    QString needle = uid.left(uid.length()- QLatin1String("@s.whatsapp.net").size());

    QList<QtContacts::QContact> contacts = contactManager.contacts();
    QString bestMatch;
    for(const QtContacts::QContact& contact : contacts)
    {
        QList<QtContacts::QContactPhoneNumber> contactPhoneNumbers = contact.details<QtContacts::QContactPhoneNumber>();
        for(const QtContacts::QContactPhoneNumber& detail : contactPhoneNumbers) {
            QString phoneNumber =  detail.number();

            sanitizePhonenumber(phoneNumber);

            //TODO: Don't do endsWith. Use the users country code to find matches if numbers
            //in telephonebook do not have a country code
            phoneNumber.remove('+');
            if(phoneNumber.startsWith("00")) //remove starting "00"
                phoneNumber = phoneNumber.mid(2);
            if(phoneNumber.startsWith("0")) //remove starting "0"
                phoneNumber = phoneNumber.mid(1);

            if(needle.endsWith(phoneNumber)) {
                QList<QtContacts::QContactDisplayLabel> labels = contact.details<QtContacts::QContactDisplayLabel>();
                if(labels.size() == 0)
                    continue;
                QString goodMatch = labels.front().label();

                if(bestMatch.length() > 0 && bestMatch != goodMatch) {
                    qDebug() << "WhosThere::getNameForUID: multiple matches " << bestMatch << " and " << goodMatch;
                }
                bestMatch = goodMatch;
            }
        }
    }
    return bestMatch;
}

void WhosThere::sanitizePhonenumber(QString& phoneNumber) {
    phoneNumber.remove('-');
    phoneNumber.remove('(');
    phoneNumber.remove(')');
    phoneNumber.remove(' ');
}

void WhosThere::onQContactManagerContactsAdded (const QList<QContactId> &contactIds ) {
    qDebug() << "WhosThere::onQContactManagerContactsAdded " << contactIds.size();
    emit addressbookReady();
}

/* --------------------------------- SimpleTextObserver ---------------------------------*/
void WhosThere::onMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr& /*channel*/) {
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
        Q_ASSERT(message.parts().size() >= 2); //header + body
        QVariantMap data;

        if(message.part(1).contains("x-whosthere-type")) {
            if(message.parts().size() > 2
               && message.part(2).contains("content")
               && message.part(2).contains("content-type")
               && message.part(2)["content-type"].variant() == "image/jpeg" )
                data["preview"] = message.part(2)["content"].variant();

            data["type"] = message.part(1)["x-whosthere-type"].variant();

            if(message.part(1).contains("x-whosthere-url"))
                data["url"] = message.part(1)["x-whosthere-url"].variant();
            if(message.part(1).contains("x-whosthere-size"))
                data["size"] = message.part(1)["x-whosthere-size"].variant();

            if(message.part(1).contains("x-whosthere-latitude"))
                data["latitude"] = message.part(1)["x-whosthere-latitude"].variant();

            if(message.part(1).contains("x-whosthere-longitude"))
                data["longitude"] = message.part(1)["x-whosthere-longitude"].variant();

            if(message.part(1).contains("x-whosthere-name"))
                data["name"] = message.part(1)["x-whosthere-name"].variant();

            if(message.part(1).contains("x-whosthere-vcard"))
                data["vcard"] = message.part(1)["x-whosthere-vcard"].variant();

        } else {
            //Text message
            Q_ASSERT(message.part(1)["content-type"].variant() == QLatin1String("text/plain"));
            data["type"] = "message";
            data["content"] = message.text();
        }

        if(message.header().contains("message-token"))
            data["msgId"] = message.header()["message-token"].variant();
        else {
            qWarning() << "message does not have message-token field!";
            return;
        }

        data["jid"] = message.sender()->id();
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

    QVariantMap data;
    data["type"] = "message";
    data["content"] = message.text();
    data["jid"] = channel->targetId();
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

void WhosThere::message_send(QString jid, QString message)
{
    static ContactMessengerPtr contactMessenger;

    if(contactMessenger.isNull() || contactMessenger->contactIdentifier() != jid) {
        qDebug() << "WhosThere::message_send: creating new ContactMessenger";
        contactMessenger = ContactMessenger::create(mAccount,jid);
        if(contactMessenger.isNull()) {
            qWarning() << "WhosThere::message_send: contactMessenger = NULL";
            return;
        }
    }
    contactMessenger->sendMessage(message);
}

/* --------------------------------- Utils ----------------------------------- */
/* Input: A international number without + or 00 */
QString WhosThere::getCountryCode(const QString& phonenumber) {
    QFile file(":countries.csv");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open :countries.csv";
        return QString();
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QList<QByteArray> fields = line.split(',');
        //Country,CountryCode[,optional data]
        QString country = *fields.begin();
        QString cc = *(fields.begin()+1);
        if(phonenumber.startsWith(cc)) {
            qDebug() << "Found country code " << cc << " from (amongst others) " << country;
            return cc;
        }
    }
    qDebug() << "Could not find country code";
    return QString();
}

void WhosThere::quit() {
    QCoreApplication::exit(0);
}

/* --------------------------------- Registration ---------------------------------*/
void WhosThere::code_request(const QString& cc, const QString& phonenumber, const QString &uid, bool useText)
{
    if(uid.length() != 32){
        emit alert("WhosThere::code_request : uid.length() != 32");
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
        emit alert("WhosThere::code_register : uid.length() != 32");
        return;
    }
    QString code = code_;
    if(code.length() == 7) //remove hyphon
        code = code.left(3) + code.right(3);
    if(code.length() != 6) {
        emit alert("WhosThere::code_register : code.length() != 6");
        return;
    }

    WhosThere::registerCode(cc, phonenumber, uid, code,
                           [this] (const QString& status, const QString& pw) {
                                emit code_register_response(status, pw);
                            });
}

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
    QString token = QCryptographicHash::hash(
                (QLatin1String("PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk1354754753509") + phoneNumber).toLatin1(),
                                     QCryptographicHash::Md5).toHex();
    urlQuery.addQueryItem("token", token);

    QUrl url("https://v.whatsapp.net/v2/code");
    url.setQuery(urlQuery);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, "WhatsApp/2.3.53 S40Version/14.26 Device/Nokia302");
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
    QString token = QCryptographicHash::hash(
                (QLatin1String("PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk1354754753509") + phoneNumber).toLatin1(),
                                     QCryptographicHash::Md5).toHex();
    urlQuery.addQueryItem("token", token);

    QUrl url("https://v.whatsapp.net/v2/register");
    url.setQuery(urlQuery);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::UserAgentHeader, "WhatsApp/2.3.53 S40Version/14.26 Device/Nokia302");
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
