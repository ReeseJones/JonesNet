#ifndef _CONNECTIONCLASS_H_
#define _CONNECTIONCLASS_H_

#include <vector>
#include <list>
#include "JNetReturn.h"
#include "Winsock2.h"
#include "Message.h"
//dont want these
#undef min
#undef max

namespace JonesNet
{

  class Connection
  {
    public:
      Connection(SOCKET* socket);
      ~Connection();
      enum State{ Connected, Disconnected};

      JNetReturn ReadMessage( Message** message ); //get a pointer to next message in the buffer.
      JNetReturn DisposeMessage( Message** message ); // dispose message after youve read it.
      JNetReturn WriteMessage( Message::Type type, void const* data, int dataSize ); // write a message
      JNetReturn Disconnect(); // stop receiving messages and disconnect from client.

      State GetStatus();

    private:
      //no one gets to call these.
      Connection( const Connection& );
      Connection& operator=(const Connection& );

      //helper functions
      JNetReturn StartReceiveThread();
      static DWORD WINAPI ReceiveThread( LPVOID lpParam );

      void AddMessagesToQue( std::vector<Message*> const & vec );

      State state_; // current state of the connection.
      std::list<Message*> messageBuffer_; // a list of received messages
      SOCKET* socket_; // socket connection to client
      Message::MessageID currentMessageID_; // just for counting recieved messages
      
      //Thread Settings
      DWORD receiveThreadID_;
      HANDLE receiveThreadHandle_;

  };

}

#endif