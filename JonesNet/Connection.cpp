
#include "Connection.h"
#include "JNet.h"
#include "MessageBuilder.h"

namespace JonesNet
{

//Connection can only be created with valid socket
Connection::Connection(SOCKET* Socket): currentMessageID_( 0 )
{
  if( socket != NULL )
  {
    int sockType;
    int result;
    int sockTypeSize = sizeof(sockType);
    result = getsockopt( *Socket, SOL_SOCKET, SO_TYPE, reinterpret_cast<char*>(&sockType), &sockTypeSize );

    if(result == SOCKET_ERROR )
    {
      printf("Failed to intialize connection with socket. Socket is in error with code: %d\n", WSAGetLastError() );
      throw JNetException( JNetException::ConnectionCreationFailed, "Passed socket found invalid." );
    }
    else // set the socket
    {
      socket_ = Socket;
      state_ = Connected;

      ULONG mode = 1;
      result = ioctlsocket( *socket_, FIONBIO, &mode );
      if(result == SOCKET_ERROR )
      {
        printf("Error attempting to set socket to non blocking: %d\n", WSAGetLastError() );
      }

      JNetReturn result = StartReceiveThread();

      if(result == JNetReturn::Success )
      {
        printf("A new connection has been established!\n");
      }
    }
  }
  else
  {
    printf("Cannot pass null socket to connection constructor.\n");
    throw JNetException( JNetException::NullSocket, "Null socket passed to connection constructor.");
  }

}

Connection::~Connection()
{
  Disconnect();
}

JNetReturn Connection::ReadMessage( Message** message ) //get a pointer to next message in the buffer.
{
  if(message == NULL)
  {
    return NullPointer;
  }
  
  if(messageBuffer_.size() > 0 )
  {
    *message = messageBuffer_.front();
    messageBuffer_.pop_front();
    return Success;
  }
  else
  {
    *message = NULL;
    return NoMessages;
  }
}

JNetReturn Connection::DisposeMessage( Message** message )
{
  if( message == NULL)
  {
    return NullPointer;
  }
  else if( *message == NULL )
  {
    return NullPointer;
  }
  else
  {
    delete *message;
    *message = NULL;
    return Success;
  }

}

JNetReturn Connection::WriteMessage( Message::Type type, void const* data, int dataSize ) // write a message
{
  if(state_ == State::Disconnected )
  {
    return JNetReturn::SocketError;
  }

  if(data == NULL ) return NoData;

  if( dataSize >= 0 )
  {
    size_t headerSize = sizeof( Message::Header );
    int totalSize = headerSize + dataSize;
    char* outputBuffer = new char[ totalSize ];
    Message::Header* messageHeader = reinterpret_cast<Message::Header*>(outputBuffer);
    char* DataStart = outputBuffer + headerSize;
    

    messageHeader->dataSize_ = dataSize;
    messageHeader->type_ = type;

    std::memcpy( DataStart, data, dataSize );

    int bytesSent = 0;

    while( bytesSent < totalSize )
    {
      int result = send( *socket_, outputBuffer + bytesSent, totalSize - bytesSent, 0 );

      if( result == SOCKET_ERROR )
      {
        int error = WSAGetLastError();

        switch( error )
        {  
          case WSAECONNRESET:
          case WSAETIMEDOUT:
          case WSAECONNABORTED:
          {
            //our connection was lost and we need to shutdown
            printf("Connection lost: %d\n", error );
            state_ = Disconnected;
          }break;

          case WSAENOBUFS:
          case WSAEWOULDBLOCK:
          {
            continue;// i think we just keep trying until it sends.
          }

          case WSAEMSGSIZE:
          {
            printf("You tried to send %d bytes. That is apparently to much\n", totalSize);
          }break;

          default:
          {
            printf("Error attempting to write message: %d\n", error );
          }break;
        }

        delete[] outputBuffer;
        return JNetReturn::SocketError;
      }
      else // no error
      {
        bytesSent += result;
      }

    }

    delete[] outputBuffer;
    return Success;
  }
  else
  {
    return JNetReturn::NoData;
  }

}

JNetReturn Connection::Disconnect() // stop receiving messages and disconnect from client.
{
  state_ = State::Disconnected;

  HANDLE getMyHandle = OpenThread( SYNCHRONIZE, false, receiveThreadID_ );

  //wait for receive thread to close and then clean up connections

  if(getMyHandle != NULL)
  {
    int val = WaitForSingleObject( getMyHandle, 1000*5 );
    
    if( val == WAIT_OBJECT_0 )
    {

    }
    else
    {
      printf("Receive thread timed out...\n");
    }

  }


  FreeSocket( & socket_ );

  return JNetReturn::Success;
}

//helper functions
JNetReturn Connection::StartReceiveThread()
{
  ReceiveThreadData* data = new ReceiveThreadData( this );
  
  if( (receiveThreadHandle_ = CreateThread( NULL, 0, Connection::ReceiveThread, data, 0, &receiveThreadID_ )) == NULL )
  {
    printf("Failed to create new thread in connection: %d", GetLastError() );

    delete data;
    return ThreadError;
  }

  return Success;
}

DWORD WINAPI Connection::ReceiveThread( LPVOID lpParam )
{
  ReceiveThreadData* data = reinterpret_cast<ReceiveThreadData*>( lpParam );
  MessageBuilder mBuilder;
  // handle reading new messages
  int const bufferSize = 4096;
  char inputBuffer[ bufferSize ];

  Connection* connection = data->GetConnection();

  //as long as we are connected.
  while( connection->GetStatus() == State::Connected )
  {
    //keep reading messages as long as we are connected.
    int bytesRead;
    

    //we are peeking here so that we can check if there is data.
    bytesRead = recv( *connection->socket_, inputBuffer, bufferSize, 0 );

    if( bytesRead == SOCKET_ERROR )
    {
      int error = WSAGetLastError();

      switch( error )
      {
          
        case WSAECONNRESET:
        case WSAETIMEDOUT:
        case WSAECONNABORTED:
        {
          //our connection was lost and we need to shutdown
          printf("Connection lost: %d\n", error );
          connection->state_ = Disconnected;
        }break;

        case WSAEWOULDBLOCK:
        {
          continue;
        }

        case WSAEMSGSIZE: // part of the message was truncated
        {
          //our message builder will handle the peices of data that it is getting.
          connection->AddMessagesToQue( mBuilder.Build( inputBuffer, bufferSize, bytesRead ) );
        }

        default:
        {
          printf("Error attempting to read data: %d\n", error );
        }break;
      }
    }
    else if( bytesRead == 0 )// no error
    {
      printf("Connection ended.\n");
      connection->state_ = Disconnected;
    }
    else // start reading in a message
    {
        connection->AddMessagesToQue( mBuilder.Build( inputBuffer, bufferSize, bytesRead ) );
    }
  }

  delete data; // delete the thread data

  return 0;
}


Connection::State Connection::GetStatus()
{
  return state_;
}

void Connection::AddMessagesToQue( std::vector<Message*> const & vec )
{
  messageBuffer_.insert( messageBuffer_.end(), vec.begin(), vec.end() );
}


}; //namespace JonesNet