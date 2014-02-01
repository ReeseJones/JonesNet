#include "JNet.h"
#include <Ws2tcpip.h>

namespace JonesNet
{

JNetReturn FreeSocket( SOCKET** socket)
{

  if(socket != NULL && *socket != NULL )
  {
    int result;
    JNetReturn ret = JNetReturn::Success;

    result = shutdown( **socket, SD_BOTH );

    if( result == SOCKET_ERROR )
    {
      printf("shutdown returned socket error code: %d\n", WSAGetLastError() );
      ret = JNetReturn::SocketError;
    }

    result = closesocket( **socket );

    if( result == SOCKET_ERROR )
    {
      printf("closesocket returned socket error code: %d\n", WSAGetLastError() );
      ret = JNetReturn::SocketError;
    }

    delete *socket;
    *socket = NULL;

    if(ret == WSAENOTCONN )
    {
      printf("Socket attemped shutdown and closesocket, but socket was not connected.\n");
    }

    return JNetReturn::Success;
  }
  else
  {
    return JNetReturn::NullSocket;
  }

}

std::string GetPrimaryIPAddress()
{
  WSADATA wsData;
  int result = WSAStartup(MAKEWORD(2,2), &wsData );

  if(result == SOCKET_ERROR )
  {
    printf("In 'GetPrimaryIPAddress' failed to start WSAStartup: %d\n", WSAGetLastError() );
  }

  const int nameBufferSize = 256;
  char hostName[nameBufferSize];

  result = gethostname( hostName, nameBufferSize );

  if(result == SOCKET_ERROR )
  {
    printf("In 'GetPrimaryIPAddress' failed to gethostname: %d\n", WSAGetLastError() );
  }

  
  hostent * hostInfo = gethostbyname( hostName );
  std::string ipAddress("127.0.0.1");

  if(hostInfo != NULL)
  {
    in_addr addr;
    std::memcpy(&addr, hostInfo->h_addr_list[0], sizeof(addr));
    ipAddress = inet_ntoa( addr );
  }
  

  WSACleanup();

  return ipAddress;
}

} //namespace JonesNet
