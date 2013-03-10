#include "telepathyclient.h"

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

    // We should always receive incoming channels of type FileTransfer, as set by our filter,
    // otherwise either MC or tp-qt itself is bogus, so let's assert in case they are
    qDebug() << "handleChannels: channelId " << chan->targetId();
    ContactPtr contact = chan->targetContact();
    qDebug() << "handleChannels: contactID " << contact->id();
    /*Q_ASSERT(chan->channelType() == TP_QT_IFACE_CHANNEL_TYPE_FILE_TRANSFER);
    Q_ASSERT(!chan->isRequested());

    IncomingFileTransferChannelPtr transferChannel = IncomingFileTransferChannelPtr::qObjectCast(chan);
    Q_ASSERT(transferChannel);*/

    context->setFinished();

    /*PendingFileReceive *receiveOperation = new PendingFileReceive(transferChannel,
            SharedPtr<RefCounted>::dynamicCast(AbstractClientPtr(this)));
    connect(receiveOperation,
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onReceiveFinished(Tp::PendingOperation*)));*/
}
