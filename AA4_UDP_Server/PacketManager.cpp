#include "PacketManager.h"
#include <iostream>
#include "EventManager.h"

void PacketManager::Init(sf::UdpSocket* _serverSocket)
{
	serverSocket = _serverSocket;

	EVENT_MANAGER.UDPSubscribe(PacketType::ACK, [this](CustomUDPPacket& packet) {

		int num = 0;
		packet.ReadVariable(num, packet.payloadOffset);
		std::cout << num << std::endl;
	});
}

void PacketManager::ProcessUDPReceivedPacket(CustomUDPPacket& customPacket)
{
	if (customPacket.udpType == UdpPacketType::CRITIC)
		EVENT_MANAGER.UDPEmit(ACK, customPacket);

	std::cout << customPacket.type << std::endl;
	EVENT_MANAGER.UDPEmit(customPacket.type, customPacket);
	
}

void PacketManager::SendPacketToClient(CustomUDPPacket& responsePacket, sf::IpAddress ipAdress, int port)
{
	if (serverSocket == nullptr)
		throw std::runtime_error("Server socket is not initialized");

	sf::Socket::Status status = serverSocket->send(responsePacket.buffer, responsePacket.bufferSize, ipAdress, port);

	if(status == sf::Socket::Status::Done)
	{
		std::cout << "Packet sent to " << ipAdress.toString() << ":" << port << std::endl;
	}
	else
	{
		std::cerr << "Failed to send packet to " << ipAdress.toString() << ":" << port << ". Error: " << static_cast<int>(status) << std::endl;
	}
}
