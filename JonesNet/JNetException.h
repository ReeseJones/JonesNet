


#ifndef _JNETEXCEPTION_H_
#define _JNETEXCEPTION_H_

#include <string>

namespace JonesNet
{

  class JNetException
  {
    public:
    enum Type
    {
      ListenerCreationFailed,
      ConnectionCreationFailed,
      ConnectionShutdownFailed,
      NullSocket
    };

    JNetException( JNetException::Type ErrorCode, const std::string& Message )
      : errorCode_( ErrorCode ), message_(Message){}

    virtual ~JNetException(){}

    Type Code(void) const { return errorCode_; }
    virtual const char * What(void) const { return message_.c_str(); }

    private:
      JNetException::Type errorCode_;
      std::string message_;
      

  };

} // namespace JonesNet

#endif