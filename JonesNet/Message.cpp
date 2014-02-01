
#include "Message.h"
#include <string>

namespace JonesNet
{

//only the message builder can construct an empty message.
Message::Message()
  :header_( Header( 0, Data ) ), data_( NULL )
{

}

Message::Message(Type type, void* data, int dataSize)
  :header_( Header( dataSize, type) ), data_( new char[dataSize] )
{
  std::memcpy( data_, data, dataSize);
}

Message::~Message()
{
  delete[] data_;
}

Message::Type Message::GetType()
{
  return header_.type_;
}

char const* Message::GetData()
{
  return data_;
}

int Message::GetSize()
{
  return header_.dataSize_;
}

} // namespace JonesNet