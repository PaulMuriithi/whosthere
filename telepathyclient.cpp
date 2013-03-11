#include "telepathyclient.h"

#include <TelepathyQt/Message>

TelepathyClient::TelepathyClient()
    : QObject(),
          AbstractClientHandler(ChannelClassSpecList() << ChannelClassSpec::textChat()
                                                       << ChannelClassSpec::serverAuthentication(),
                  AbstractClientHandler::Capabilities(), false)
{
}

TelepathyClient::~TelepathyClient() {

}

void TelepathyClient::handleChannels(const MethodInvocationContextPtr<> &context,
        const AccountPtr &account,
        const ConnectionPtr &connection,
        const QList<ChannelPtr> &channels,
        const QList<ChannelRequestPtr> &requestsSatisfied,
        const QDateTime &userActionTime,
        const HandlerInfo &handlerInfo)
{
    qDebug() << "TelepathyClient::handleChannels";
    // We should always receive one channel to handle,
    // otherwise either MC or tp-qt itself is bogus, so let's assert in case they are
    Q_ASSERT(channels.size() == 1);
    ChannelPtr chan = channels.first();

    if (!chan->isValid()) {
        qWarning() << "Channel received to handle is invalid, ignoring channel";
        context->setFinishedWithError(TP_QT_ERROR_INVALID_ARGUMENT,
                QLatin1String("Channel received to handle is invalid"));
        return;
    }

    //ContactPtr contact = chan->targetContact();
    //QString jid = contact->id();
    //qDebug() << "handleChannels: contactID " << jid;
    /*if(chan->channelType() != TP_QT_IFACE_CHANNEL_TYPE_TEXT)
        return;
    */
    TextChannelPtr textChannel = TextChannelPtr::qObjectCast(chan);
    connect(textChannel.data(), &TextChannel::messageReceived,
            [this, textChannel](const ReceivedMessage& r) {
                emit messageReceived(r, textChannel);
        });
    connect(textChannel.data(), &TextChannel::messageSent,
            [this, textChannel](const Tp::Message& message, Tp::MessageSendingFlags flags, const QString& msgId) {
                emit messageSent(message, flags, msgId, textChannel);
        });
    context->setFinished();

    /*PendingFileReceive *receiveOperation = new PendingFileReceive(transferChannel,
            SharedPtr<RefCounted>::dynamicCast(AbstractClientPtr(this)));
    connect(receiveOperation,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onReceiveFinished(Tp::PendingOperation*)));*/
}
