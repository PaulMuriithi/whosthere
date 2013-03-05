#include "telepathyclient.h"

TelepathyClient::TelepathyClient()
    : QObject(),
          AbstractClientHandler(ChannelClassSpecList() << ChannelClassSpec::incomingFileTransfer() <<
                                                          ChannelClassSpec::textChat(),
                  AbstractClientHandler::Capabilities(), false)
{}

TelepathyClient::~TelepathyClient() {}
