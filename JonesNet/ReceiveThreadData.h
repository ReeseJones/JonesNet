#ifndef _RECEIVETHREADDATACLASS_H_
#define _RECEIVETHREADDATACLASS_H_

namespace JonesNet
{
  class Connection;

  class ReceiveThreadData
  {
    public:
    ReceiveThreadData(Connection* connection): connection_( connection ){}
    //~ReceiveThreadData();

    Connection* GetConnection() const { return connection_; }

    private:
    Connection* connection_;
  };

} //namespace JonesNet

#endif