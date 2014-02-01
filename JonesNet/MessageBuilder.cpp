
#include "MessageBuilder.h"
#include <string>



namespace JonesNet
{

  MessageBuilder::MessageBuilder() : messageHeader_( Message::Header(0, Message::Data) ),
    messageSeekLocation_(0), dataBuffer_(NULL), messageInProgress_(false)
  {

  }

  std::vector<Message*> const& MessageBuilder::Build(const char *dataBuffer, int bufferSize, int bytesRead)
  {
    //3 cases
    // CASE 1: This is the exact amount of data we need to finish message or greater
    // CASE 2: There is not enough data to finish the message

    messages_.clear();
    messages_.reserve( 8 );

    const char *dataBufferSeek = dataBuffer;
    size_t headerSize = sizeof(Message::Header);

    do
    {
      //if we have a message construction in progress
      if( !messageInProgress_ ) // if there is no message in progress, read headers
      {
        //check that we have read enough enough bits for a header
        if( bytesRead >= sizeof(Message::Header) )
        {
          //we have started to read a message!
          messageInProgress_ = true;

          //***********GET MESSAGE HEADER *******************//
          Message::Header const *messageHeader = reinterpret_cast< Message::Header const *>(dataBufferSeek);
          messageHeader_ = *messageHeader;
          dataBufferSeek += headerSize;
          bytesRead -= headerSize;

          //************CREATE DATA BUFFER ******************//
          //create data buffer for new message.
          dataBuffer_ = new char[messageHeader_.dataSize_];
          //Set the data seek
          messageSeekLocation_ = 0;
        }
        else
        {
          printf("Cannot start new message with %d bytes\n", bytesRead ); //TODO FIX THIS
        }
      }
      //may or may not have just read header, but now we need to read the data
      
      int dataBytesLeft = messageHeader_.dataSize_ - messageSeekLocation_;
      //case 1
      if(bytesRead >= dataBytesLeft )
      {
        //copy from the recv buffer
        std::memcpy( dataBuffer_ + messageSeekLocation_, dataBufferSeek, dataBytesLeft );
        dataBufferSeek += dataBytesLeft;
        bytesRead -= dataBytesLeft;

        //add message to list
        messages_.push_back( BundleAndSend() );
      }
      else //if( bytesRead < dataBytesLeft) // case 2
      {
        //copy what we got
        std::memcpy( dataBuffer_ + messageSeekLocation_, dataBufferSeek, bytesRead );

        messageSeekLocation_ += bytesRead;
        dataBufferSeek += bytesRead;
        bytesRead -= bytesRead;
        //message didnt finish, but all bytes are read.
      }

    }while( bytesRead > 0);

    return messages_;
  }

  Message* MessageBuilder::BundleAndSend()
  {
    Message* message = new Message();
    message->header_ = messageHeader_;
    message->data_ = dataBuffer_;

    messageSeekLocation_ = 0;
    dataBuffer_ = NULL;
    messageInProgress_ = false;

    return message;
  }

}//namespace JonesNet