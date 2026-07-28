#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H
#include <cstddef>

namespace protocol {
    inline constexpr std::size_t MAX_FRAME_SIZE = 64 * 1024; // 64 KB
}
#endif 
