
#include "nakedmobile.hpp"

#include <world/manager.hpp>

namespace uome {
namespace net {
namespace packets {

NakedMobile::NakedMobile() : Packet(0x77, 17) {
}

bool NakedMobile::read(const int8_t* buf, unsigned int len, unsigned int& index) {
    bool ret = true;

    ret = ret && PacketReader::read(buf, len, index, serial_);
    ret = ret && PacketReader::read(buf, len, index, bodyId_);
    ret = ret && PacketReader::read(buf, len, index, locX_);
    ret = ret && PacketReader::read(buf, len, index, locY_);
    ret = ret && PacketReader::read(buf, len, index, locZ_);
    ret = ret && PacketReader::read(buf, len, index, direction_);
    ret = ret && PacketReader::read(buf, len, index, hue_);
    ret = ret && PacketReader::read(buf, len, index, status_);
    ret = ret && PacketReader::read(buf, len, index, notoriety_);

    return ret;
}

void NakedMobile::onReceive() {
    world::Manager::getSingleton()->handleNakedMobile(this);
}

}
}
}
