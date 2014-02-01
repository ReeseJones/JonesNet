#ifndef _JNETRETURNENUM_H_
#define _JNETRETURNENUM_H_

namespace JonesNet
{

  enum JNetReturn
  {
    Success,
    SocketError,
    ThreadError,
    NullSocket,
    NullPointer,
    AlreadyListening,
    NoMessages,
    NoData,
  };
 
}

#endif;