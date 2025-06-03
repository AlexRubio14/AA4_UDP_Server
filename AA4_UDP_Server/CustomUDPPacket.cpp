#include "CustomUDPPacket.h"
#include <iostream>

CustomUDPPacket::CustomUDPPacket(UdpPacketType udpType, PacketType type, int playerId)
{
    bufferSize = 0;
	payloadOffset = 0;

    this->type = type;
    this->udpType = udpType;
    this->playerId = playerId;

    WriteVariable(static_cast<uint8_t>(udpType));
    WriteVariable(static_cast<uint8_t>(type));
    WriteVariable(playerId);

    payloadOffset = bufferSize;
}

void CustomUDPPacket::ReadBuffer(const char* inputBuffer, size_t _bufferSize)
{
    size_t offset = 0;

    // Copiamos todo el paquete al buffer interno, incluyendo cabecera y payload
    std::memcpy(buffer, inputBuffer, _bufferSize);
    bufferSize = _bufferSize;

	uint8_t _udpType = 0;
	uint8_t _type = 0;

    ReadVariable(_udpType, offset);
    ReadVariable(_type, offset);
    ReadVariable(playerId, offset);

	udpType = static_cast<UdpPacketType>(_udpType);
	type = static_cast<PacketType>(_type);

    payloadOffset = offset;
}

bool CustomUDPPacket::WriteString(const std::string& str)
{
	// Check if there is enough space in the buffer for the lenght data
    int length = static_cast<int>(str.size());
    if (!WriteVariable(length))
        return false;

	// Check if there is enough space in the buffer for the string data
    if (bufferSize + length > sizeof(buffer))
        return false;

	// Copy the string data into the buffer
    std::memcpy(buffer + bufferSize, str.data(), length);
    bufferSize += length;

    return true;
}

bool CustomUDPPacket::ReadString(std::string& outStr, size_t& offset) const {
    int length = 0;

	// Read size of the string
    if (!ReadVariable(length, offset))
        return false;

	// Check if the length is valid
    if (offset + length > bufferSize)
        return false;

    // Read the string data
    outStr.assign(buffer + offset, length);
    offset += length;

    return true;
}