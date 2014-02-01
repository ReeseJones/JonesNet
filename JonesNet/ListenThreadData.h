#ifndef _LISTENTHREADDATACLASS_H_
#define _LISTENTHREADDATACLASS_H_

namespace JonesNet
{
  class Listener;

  class ListenThreadData
  {
    public:
    ListenThreadData(Listener* listener): listener_( listener ){}
    //~ListenThreadData();

    Listener* GetListener() const { return listener_; }

    private:
    Listener* listener_;
  };

} //namespace JonesNet

#endif