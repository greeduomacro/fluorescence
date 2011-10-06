#ifndef FLUO_NET_PACKETS_DOUBLECLICK_HPP
#define FLUO_NET_PACKETS_DOUBLECLICK_HPP

#include <net/packet.hpp>

#include <typedefs.hpp>

namespace fluo {
namespace net {

namespace packets {

class DoubleClick : public Packet {
public:
    DoubleClick(Serial serial);

    virtual bool write(int8_t* buf, unsigned int len, unsigned int& index) const;

private:
    Serial serial_;

};

}
}
}

#endif