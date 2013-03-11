#ifndef TELEPATHYCLIENT_H
#define TELEPATHYCLIENT_H

#include <QObject>
#include <TelepathyQt/Types>
#include <TelepathyQt/Account>
#include <TelepathyQt/AbstractClientHandler>
#include <TelepathyQt/ChannelClassSpec>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/IncomingFileTransferChannel>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/TextChannel>

using namespace Tp;

class TelepathyClient : public QObject, public AbstractClientHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(TelepathyClient)
public:
    static SharedPtr<TelepathyClient> create()
    {
        return SharedPtr<TelepathyClient>(new TelepathyClient());
    }

    ~TelepathyClient();
    bool bypassApproval() const { return true; }

    void handleChannels(const MethodInvocationContextPtr<> &context,
            const AccountPtr &account,
            const ConnectionPtr &connection,
            const QList<ChannelPtr> &channels,
            const QList<ChannelRequestPtr> &requestsSatisfied,
            const QDateTime &userActionTime,
            const HandlerInfo &handlerInfo);
signals:
    void messageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &channel);
    void messageSent (const Tp::Message& message,
                                    Tp::MessageSendingFlags flags,
                                    const QString& msgId,
                                    const Tp::TextChannelPtr& channel);
private Q_SLOTS:

private:
    TelepathyClient();

    QSet<PendingOperation*> mReceiveOps;
};

#endif // TELEPATHYCLIENT_H