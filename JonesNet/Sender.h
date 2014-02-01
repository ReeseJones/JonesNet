#ifndef _SENDERCLASS_H_
#define _SENDERCLASS_H_

#include "JNetReturn.h"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")

namespace JonesNet
{
  class Connection;
  
  class Sender
  {
    public:
    Sender( std::string IPAddress );
    ~Sender();
    JNetReturn Connect( std::string IPAddress, short port );
    JNetReturn Disconnect();
    Connection* GetConnection();

    private:
    Connection* connection_;
    std::string IPAddress_;
    WSADATA wsData_;
    

  };

}

#endif