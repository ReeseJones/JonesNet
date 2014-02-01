#ifndef _LISTENERCLASS_H_
#define _LISTENERCLASS_H_


#define WIN32_LEAN_AND_MEAN
#include <list>
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#include "JNetReturn.h"
#include "Message.h"

namespace JonesNet
{
  class Connection;

  class Listener
  {
    public:
    Listener(std::string IPAddress = std::string("127.0.0.1"), int port = 0, int maxConnections = 10);
    ~Listener();

    //start or stop listener
    JNetReturn Start();
    JNetReturn StopListening();

    //Gettors for listener data
    short GetPort();
    std::string GetIP();

    //close all connections
    JNetReturn CloseConnections();

    //referance the list of the connections
    std::list<Connection*>& GetConnections();

    //writes a message ot all connections
    JNetReturn WriteToAll( Message::Type type, void const* data, int dataSize );


    private:
    //no one gets to call these.
    Listener( const Listener& );
    Listener& operator=(const Listener& );

    //helper functions
    JNetReturn StartListenThread();
    static DWORD WINAPI ListenThread( LPVOID lpParam );
    JNetReturn StartListenSocket();

    JNetReturn CloseListener();

    //Listener settings
    int maxConnections_;
    bool isListening_;
    std::list<Connection*> connections_;
    
    //Connection settings
    short listenPort_;
    sockaddr_in socketAddress_;
    std::string IPAddress_;

    //Thread Settings
    DWORD listenThreadID_;

    //listener sockets
    SOCKET socket_;

    //Winsock Data
    WSADATA wsData_;
      

  };

}

#endif