#pragma once

enum class UdpPacketType : uint8_t { NORMAL = 0b00000000, URGENT = 0b00000001, CRITIC = 0b00000010 };


inline UdpPacketType operator|(UdpPacketType lhs, UdpPacketType rhs) {
    return static_cast<UdpPacketType>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline UdpPacketType& operator|=(UdpPacketType& lhs, UdpPacketType rhs) {
    lhs = lhs | rhs;
    return lhs;
}

inline UdpPacketType operator&(UdpPacketType lhs, UdpPacketType rhs) {
    return static_cast<UdpPacketType>(
        static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
        );
}

inline UdpPacketType& operator&=(UdpPacketType& lhs, UdpPacketType rhs) {
    lhs = lhs & rhs;
    return lhs;
}