#pragma once
#include <SFML/Network.hpp>
#include "PacketType.h"
#include "UDPPacketType.h"

class CustomUDPPacket
{
public:
    CustomUDPPacket() = default;
    CustomUDPPacket(UdpPacketType udpType, PacketType type);

    void ReadBuffer(const char* inputBuffer, size_t _bufferSize);

    PacketType type;
    UdpPacketType udpType;

    char buffer[1024];
    size_t bufferSize;

    size_t payloadOffset;

	// This method let the user to write the variable data into the buffer automatically 
    template<typename T>
    bool WriteVariable(const T& data)
    {
        if (bufferSize + sizeof(T) > sizeof(buffer)) {
			// Buffer overflow, don't write data
            return false;
        }
        std::memcpy(buffer + bufferSize, &data, sizeof(T));
        bufferSize += sizeof(T);
        return true;
    }

	// This method let the user read the next varaible in the buffer and set the value to the outData variable

    template<typename T>
    bool ReadVariable(T& outData, size_t& offset) const {
        if (offset + sizeof(T) > bufferSize)
            return false; // There is no data to read

        std::memcpy(&outData, buffer + offset, sizeof(T));
        offset += sizeof(T);
        return true;
    }

    bool WriteString(const std::string& str);
};
