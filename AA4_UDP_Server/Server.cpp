#include "Server.h"
#include <optional>
#include "CustomUDPPacket.h"
#include "PacketManager.h"
#include "CriticalPacketManager.h"
Server::Server()
{
	socket = std::make_unique<sf::UdpSocket>();
	receivedBufferSize = 0;
	isRunning = false;
}

Server::~Server()
{
}

void Server::Start()
{
	if (socket->bind(LISTENER_PORT) != sf::Socket::Status::Done)
	{
		std::cerr << "Error binding to port " << LISTENER_PORT << std::endl;
		isRunning = false;
		return;
	}

	std::cout << "Server started on port " << LISTENER_PORT << std::endl;

	// Setup own variables
	socket->setBlocking(false);
	isRunning = true;

	// Setup Managers
	PACKET_MANAGER.Init(socket.get());
	CRITICAL_PACKET_MANAGER.StartCriticalPacketUpdateThread();
}

void Server::Update()
{
	while (true)
	{
		std::optional<sf::IpAddress> senderIP;
		unsigned short port;

		if (socket->receive(serverBuffer, sizeof(serverBuffer), receivedBufferSize, senderIP, port) == sf::Socket::Status::Done)
		{
			//std::cout << "Received packet from " << senderIP.value() << " : " << port << std::endl;
			CustomUDPPacket customUDPPacket;
			customUDPPacket.ReadBuffer(serverBuffer, receivedBufferSize);

			//Packet Manager procesa el paquete
			PACKET_MANAGER.ProcessUDPReceivedPacket(customUDPPacket, *senderIP, port);


			ClearBuffer();
		}
	}
}

void Server::ClearBuffer()
{
	std::memset(serverBuffer, 0, sizeof(serverBuffer));
	receivedBufferSize = 0;
}
