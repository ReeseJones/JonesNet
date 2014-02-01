
#include "Sender.h"
#include "JNet.h"

namespace JonesNet
{

  Sender::Sender( std::string IPAddress )
    :IPAddress_(IPAddress), connection_(NULL)
  {
    WSAStartup( MAKEWORD(2,2), &wsData_ );
  }

  Sender::~Sender()
  {

    WSACleanup();
  }

  JNetReturn Sender::Connect( std::string IPAddress, short port )
  {
    SOCKET* newConnection = new SOCKET();
    sockaddr_in senderAddress;
    senderAddress.sin_family = AF_INET;
    senderAddress.sin_port = 0;
    senderAddress.sin_addr.S_un.S_addr = inet_addr( IPAddress_.c_str() );

    sockaddr_in remoteAddress;
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons( port );
    remoteAddress.sin_addr.S_un.S_addr = inet_addr( IPAddress.c_str() );

    *newConnection = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    if( *newConnection == INVALID_SOCKET )
    {
      printf("Error, invalid socket, error code: %d\n", WSAGetLastError() );
      delete newConnection;
      newConnection = NULL;
      return JNetReturn::SocketError;
    }

    //socket was created fine, time to bind it.

    int result = bind( *newConnection, reinterpret_cast<sockaddr*>( &senderAddress), sizeof(senderAddress) );

    if( result == SOCKET_ERROR )
    {
      printf("Error binding socket to local endpoint. Error code: %d\n", WSAGetLastError() );
      FreeSocket( &newConnection );
      return JNetReturn::SocketError;
    }

    result = connect( *newConnection, reinterpret_cast<sockaddr*>( &remoteAddress), sizeof(remoteAddress) );

    if(result == SOCKET_ERROR )
    {
      result = WSAGetLastError();
      printf("Failed to connect. Error code: %d\n", result );
      FreeSocket( &newConnection );
      return JNetReturn::SocketError;
    }

    //last step create the connection
    try
    {
      connection_ = new Connection( newConnection );
    }
    catch( JNetException const & exception )
    {
      printf("Failed to create a connection object.\n");
      printf("Error code: %d\nError: %s\n", exception.Code(), exception.What() );
      FreeSocket( &newConnection );
    }

    return JNetReturn::Success;

  }

  JNetReturn Sender::Disconnect()
  {
    if(connection_ != NULL )
    {
      delete connection_;
      connection_ = NULL;

      return JNetReturn::Success;
    }

    return JNetReturn::NullPointer;
  }

  Connection* Sender::GetConnection()
  {
    return connection_;
  }

}; //namespace JonesNet