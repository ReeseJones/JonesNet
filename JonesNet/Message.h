#ifndef _MESSAGECLASS_H_
#define _MESSAGECLASS_H_

namespace JonesNet
{

  class Message
  {
    public:
      typedef unsigned long MessageID;
      enum Type{Data, Error, Warning, MaxConnections};

      struct Header
      {
        Header(int DataSize, Type MessageType):dataSize_(DataSize), type_( MessageType){}
        int dataSize_;
        Type type_;
      };

      Message(Type type, void* data, int dataSize);
      ~Message();

      Type GetType();
      char const * GetData();
      int GetSize();
    

    private:
      friend class MessageBuilder;
      Message();
      Header header_;
      char* data_;

  };

}

#endif