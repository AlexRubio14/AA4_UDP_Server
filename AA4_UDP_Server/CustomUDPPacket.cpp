#include "CustomUDPPacket.h"
#include <iostream>

CustomUDPPacket::CustomUDPPacket(UdpPacketType udpType, PacketType type)
	: udpType(udpType), type(type), bufferSize(0) {}

void CustomUDPPacket::ReadBuffer(const char* inputBuffer, size_t _bufferSize)
{
	size_t offset = 0;

	// Copiamos todo el paquete al buffer interno, incluyendo cabecera y payload
	std::memcpy(buffer, inputBuffer, _bufferSize);
	bufferSize = _bufferSize;

	ReadVariable(udpType, offset);
	ReadVariable(type, offset);

	payloadOffset = offset; 

	std::cout << "UdpType: " << static_cast<int>(udpType) << " Type: " << static_cast<int>(type) << std::endl;
}

bool CustomUDPPacket::WriteString(const std::string& str)
{
	uint16_t length = static_cast<uint16_t>(str.size());
	// Primero escribimos el tamaño
	if (!WriteVariable(length))
		return false;

	// Ahora escribimos los caracteres (sin '\0')
	if (bufferSize + length > sizeof(buffer))
		return false;

	std::memcpy(buffer + bufferSize, str.data(), length);
	bufferSize += length;

	return true;
}