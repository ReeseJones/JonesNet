
#include "Listener.h"
#include "JNet.h"

namespace JonesNet
{

Listener::Listener(std::string IPAddress, int port, int maxConnections):
  IPAddress_(IPAddress), listenPort_( port ), maxConnections_( maxConnections ),
  isListening_( false )
{
  int result;

  //startup winsock
  result = WSAStartup( MAKEWORD(2,2), &wsData_ );

  if( result != 0 )
  {
    printf("Error statring winsock: %d\n", result );
    WSACleanup();
    throw JNetException( JNetException::ListenerCreationFailed, "Failed to start winsock." );
  }

  //settup socket address
  socketAddress_.sin_family = AF_INET;
  socketAddress_.sin_port = htons( port );
  socketAddress_.sin_addr.S_un.S_addr = inet_addr( IPAddress_.c_str() );
  
}

Listener::~Listener()
{

  //shutdown listening thread
  //shutdown listening socket
  StopListening();

  //cleanup
  WSACleanup();
}

//start or stop listener
JNetReturn Listener::Start() //start listening on listen thread
{
  
  if( StartListenSocket() == JNetReturn::Success )
  {
  

  JNetReturn jnResult = StartListenThread();////-------------------TODO FIX

  if(jnResult == Success )
  {
    isListening_ = true;
  }

  return jnResult;

  }
  else
  {
    return JNetReturn::SocketError;
  }
}

JNetReturn Listener::StopListening()
{
  JNetReturn returnVal;
  isListening_ = false;

  //wait for listen thread to close and then clean up connections
  WaitForSingleObject( OpenThread(SYNCHRONIZE, false, listenThreadID_ ), INFINITE );

  //closes the listener socket.
  returnVal = CloseListener();

  return returnVal;

  //should now be able to be started again.

  //NOTE: THIS DOES NOT CLOSE DOWN CONNECTIONS THAT HAVE BEEN MADE!

}

JNetReturn Listener::StartListenSocket()
{

  if(isListening_ == true )
  {
    return JNetReturn::AlreadyListening;
  }

  int result;

  //create socket
  socket_ = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );

  if( socket_ == INVALID_SOCKET )
  {
    printf("WSASocket call failed with error: %d\n", WSAGetLastError() );
    throw JNetException( JNetException::ListenerCreationFailed, "Failed to create socket." );
  }

  //bind address to socket
  result = bind( socket_, reinterpret_cast<SOCKADDR*>(&socketAddress_), sizeof( socketAddress_ ) );

  if(result == SOCKET_ERROR )
  {
    printf("Error binding socket address to socket: %d\n", WSAGetLastError() );
    throw JNetException( JNetException::ListenerCreationFailed, "Failed to bind socket." );
  }

  result = listen( socket_, maxConnections_ );

  if(result == SOCKET_ERROR )
  {
    printf("Error setting socket to listen mode: %d\n", WSAGetLastError() );
    return SocketError;
  }

  ULONG mode = 1;
  result = ioctlsocket( socket_, FIONBIO, &mode );
  if(result == SOCKET_ERROR )
  {
    printf("Error attempting to set socket to non blocking: %d\n", WSAGetLastError() );
    return SocketError;
  }
   

  return JNetReturn::Success;
}

//Gettors for listener data
short Listener::GetPort()
{
  return listenPort_;
}

std::string Listener::GetIP()
{
  return IPAddress_;
}

JNetReturn Listener::StartListenThread()
{
  ListenThreadData* data = new ListenThreadData( this );
  
  if( CreateThread( NULL, 0, Listener::ListenThread, data, 0, &listenThreadID_ ) == NULL )
  {
    printf("Failed to create new thread: %d", GetLastError() );

    delete data;
    return ThreadError;
  }

  return Success;
}

DWORD WINAPI Listener::ListenThread( LPVOID lpParam )
{
  ListenThreadData* data = reinterpret_cast<ListenThreadData*>( lpParam );
  sockaddr_in remoteEndpoint;
  int endpointSize = sizeof( remoteEndpoint );

  SOCKET* tentativeClientSocket = new SOCKET();

  while( data->GetListener()->isListening_ )
  {
    int acceptReturnVal;
    
    (*tentativeClientSocket) = accept( data->GetListener()->socket_, reinterpret_cast<sockaddr*>(&remoteEndpoint), &endpointSize );
    acceptReturnVal = WSAGetLastError();

    if( (*tentativeClientSocket) == INVALID_SOCKET )
    {
      if(acceptReturnVal == WSAEWOULDBLOCK)
      {
        //continue if accept would block
        continue;
      }
      else
      {
        printf("accept returned invalid socket with error code: %d\n", WSAGetLastError() );
      }
    }
    else // a connection was accepted.
    {
     // create a connection object and pass it on to the listener
     Connection *connection;
      try
      {
        connection = new Connection( tentativeClientSocket );

        if( connection != NULL )
        {
          //connection created succesfully. push it to the list!
          data->GetListener()->connections_.push_back( connection );

          //connection created just fine.
          //create a new socket for the next time we need it.
          tentativeClientSocket = new SOCKET();

          if(data->GetListener()->connections_.size() == data->GetListener()->maxConnections_)
          {
            data->GetListener()->isListening_ = false;
          }

        }
        else
        {
          printf("Connection NULL\n");
        }
      }
      catch( JNetException const & exception )
      {
        printf("code:%d\nmessage: %s\n", exception.Code(), exception.What() );

        switch( exception.Code() )
        {
          case JNetException::ConnectionCreationFailed:
          {
            printf("Attempting to free socket...\n");
            FreeSocket( &tentativeClientSocket );
          }break;

        } //end switch
      } //end catch
    }   //end else   
  } // end while loop  

  //delete reserved socket
  delete tentativeClientSocket;

  //delete data given to us at start of this thread.
  delete data;

  return 0;
}

JNetReturn Listener::CloseListener()
{
  int result;

  result = shutdown( socket_, SD_BOTH );

  if(result == SOCKET_ERROR )
  {
    return JNetReturn::SocketError;
  }

  result = closesocket( socket_ );

  if(result == SOCKET_ERROR )
  {
    return JNetReturn::SocketError;
  }

  return JNetReturn::Success;

}

JNetReturn Listener::CloseConnections()
{
  std::list<Connection*>::iterator it = connections_.begin();

  while( it != connections_.end() )
  {
    //end all connections
    delete *it;
    ++it;
  }

  //clear the list of connections.
  connections_.clear();

  return Success;

}

std::list<Connection*>& Listener::GetConnections()
{
  return connections_;
}

JNetReturn Listener::WriteToAll( Message::Type type, void const* data, int dataSize )
{
  std::list<JonesNet::Connection*>::iterator it = connections_.begin();

  while( it != connections_.end() )
  {
    (*it)->WriteMessage( type, data, dataSize ); 
     ++it;
  }

  return Success;
}

} // namespace JonesNet