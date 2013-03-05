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
#include <TelepathyQt/Debug>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/AccountFactory>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/ContactMessenger>

#include "whosthere.h"
#include "telepathyclient.h"

using namespace std;

WhosThere::WhosThere(QQuickItem *parent) :
    QQuickItem(parent)
{

    AccountFactoryPtr accountFactory = AccountFactory::create(QDBusConnection::sessionBus(),
        Account::FeatureCore);

    ConnectionFactoryPtr connectionFactory = ConnectionFactory::create(QDBusConnection::sessionBus(),
                Connection::FeatureConnected | Connection::FeatureConnected |
                Connection::FeatureRoster | Connection::FeatureRosterGroups);

    ChannelFactoryPtr channelFactory = ChannelFactory::create(QDBusConnection::sessionBus());

    ContactFactoryPtr contactFactory = ContactFactory::create(Contact::FeatureAlias | Contact::FeatureSimplePresence);

    mAM = Tp::AccountManager::create(accountFactory, connectionFactory, channelFactory, contactFactory);

    /*mCR = ClientRegistrar::create(accountFactory, connectionFactory,
                channelFactory);

    mHandler = TelepathyClient::create();
    QString handlerName(QLatin1String("TpQtExampleFileReceiverHandler"));
    if (!mCR->registerClient(AbstractClientPtr::dynamicCast(mHandler), handlerName)) {
        qWarning() << "Unable to register incoming file transfer handler, aborting";
    }*/
    /*cm = ConnectionManager::create("yowsup");
    connect(mCM->becomeReady(),
                SIGNAL(finished(Tp::PendingOperation *)),
                SLOT(onCMReady(Tp::PendingOperation *)));*/
    connect(mAM->becomeReady(), &PendingOperation::finished,
            this, &WhosThere::onAMReady);
    connect(mAM.data(),
                SIGNAL(newAccount(const Tp::AccountPtr &)),
                SLOT(onNewAccount(const Tp::AccountPtr &)));
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

    QList< AccountPtr >  accounts = mAM->accountsByProtocol("whatsapp")->accounts();
    qDebug() << "number of accounts: " << accounts.size();
    if(accounts.size() == 0) {
        qDebug() << "Creating new account";
        QVariantMap parameters;
        parameters.insert( "password", QVariant("C7+4QINyzdXK7iq6wAEMhqwcjIQ="));
        parameters.insert( "phonenumber", QVariant("14155280162"));
        QVariantMap properties;
        properties.insert( "org.freedesktop.Telepathy.Account.Enabled", true );
        PendingAccount* acc = mAM->createAccount("yowsup", "whatsapp", "1. WhatApp Account", parameters, properties);

        connect(acc, &PendingAccount::finished,
                this, &WhosThere::onAccountCreateFinished );
    } else {
        mAccount = *accounts.begin();
        connect(mAccount->becomeReady(), &PendingOperation::finished,
                this, &WhosThere::onAccountFinished);
    }
}

void WhosThere::onAccountCreateFinished(PendingOperation* op) {
    if (op->isError()) {
        qWarning() << "Account cannot become created -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }
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
    //qDebug() << "WhosThere::setAccount nickname: " << account->cmName();
    qDebug() << "WhosThere::setAccount valid : " << mAccount->isValidAccount() << " enabled: " << mAccount->isEnabled();
    if(!mAccount->isEnabled()) //FIXME
        connect(mAccount->setEnabled(true), &PendingOperation::finished,
                this, &WhosThere::onAccountSetEnabled);

    SimpleTextObserverPtr m_simpleTextObserver = SimpleTextObserver::create(mAccount);
    connect(m_simpleTextObserver.data(), &SimpleTextObserver::messageReceived,
            this, &WhosThere::onMessageReceived);
    //mAccount->setConnectsAutomatically(true); //FIXME
    connect(mAccount.data(), &Account::connectionChanged,
            this, &WhosThere::onAccountConnectionChanged);
    onAccountConnectionChanged(mAccount->connection());
}

void WhosThere::onMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &channel) {
    qDebug() << "WhosThere::onMessageReceived isDeliveryReport: " << message.isDeliveryReport()
             << " text: " << message.text()
             << " sender: " << message.sender()->id();
}

void WhosThere::onAccountSetEnabled(PendingOperation* acc) {
    if (acc->isError()) {
        qWarning() << "Account cannot become set enabled -" <<
            acc->errorName() << ": " << acc->errorMessage();
        return;
    }
}

void WhosThere::onAccountConnectionChanged(const ConnectionPtr &conn)
{
    if (conn) {
        qDebug() << "WhosThere::onAccountConnectionChanged";
        mConn = conn;
        emit auth_success("");
        connect(mConn->contactManager().data(), &ContactManager::stateChanged,
                this, &WhosThere::onContactManagerStateChanged);
        onContactManagerStateChanged(mConn->contactManager()->state());
    } else {
        qDebug() << " WhosThere::onAccountConnectionChanged: conn = NULL";
    }
}

void WhosThere::onContactManagerStateChanged(ContactListState state)
{
    qDebug() << "WhosThere::onContactManagerStateChanged " << state;
    if (state == ContactListStateSuccess) {
        qDebug() << "Loading contacts";
        foreach (const ContactPtr &contact, mConn->contactManager()->allKnownContacts()) {
            qDebug() << "Contact id: " << contact->id();
        }
    }
}
/*
void WhosThere::onCMReady(PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "CM" << cm->name() << "cannot become ready -" <<
            op->errorName() << ": " << op->errorMessage();
        return;
    }

    qDebug() << "CM" << cm->name() << "ready!";
    qDebug() << "Supported protocols:";
    ProtocolInfo pi = mCM->protocol("whatsapp");

    connect(conn->contactManager().data(),
            SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
            SLOT(onPresencePublicationRequested(const Tp::Contacts &)));
    // TODO listen to allKnownContactsChanged

    connect(conn->contactManager().data(),
            SIGNAL(stateChanged(Tp::ContactListState)),
            SLOT(onContactManagerStateChanged(Tp::ContactListState)));
}*/


void WhosThere::message_send(QString jid, QByteArray message)
{
    ContactMessengerPtr contactMessenger = ContactMessenger::create(mAccount, jid);
    if(!contactMessenger) {
        qDebug() << "WhosThere::message_send: contactMessenger = NULL";
        return;
    }
    contactMessenger->sendMessage(message);
}

void WhosThere::login(const QString& username, const QByteArray& password)
{}
void WhosThere::disconnect(const QByteArray& reason)
{}

void WhosThere::code_register(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &code, const QByteArray &identity)
{}
void WhosThere::code_request(const QByteArray &countryCode, const QByteArray &phoneNumber, const QByteArray &identity, bool useText)
{}

