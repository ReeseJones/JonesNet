
#ifndef _JNET_H_
#define _JNET_H_

#include "Listener.h"
#include "Sender.h"
#include "Connection.h"
#include "Message.h"
#include "ListenThreadData.h"
#include "ReceiveThreadData.h"
#include "JNetException.h"
#include "JNetReturn.h"

namespace JonesNet
{

JNetReturn FreeSocket( SOCKET** socket ); // Free a socket allocated by new

std::string GetPrimaryIPAddress();

} // namespace JonesNet

#endif