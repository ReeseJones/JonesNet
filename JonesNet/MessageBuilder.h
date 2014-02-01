#ifndef _MESSAGEBUILDER_H_
#define _MESSAGEBUILDER_H_

#include "Message.h"
#include <vector>

namespace JonesNet
{

  class MessageBuilder
  {
    public:
      MessageBuilder();
      std::vector<Message*> const& Build(const char *dataBuffer, int bufferSize, int bytesRead);

    private:
      Message* BundleAndSend();

      Message::Header messageHeader_;
      int messageSeekLocation_;
      char* dataBuffer_;
      bool messageInProgress_;
      std::vector<Message*> messages_;

  };
}//namespace JonesNet


#endif