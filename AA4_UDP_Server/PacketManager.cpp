#include "PacketManager.h"
#include <iostream>
#include "EventManager.h"
#include "RoomManager.h"
#include "CriticalPacketManager.h"

void PacketManager::Init(sf::UdpSocket* _serverSocket)
{
	serverSocket = _serverSocket;

	EVENT_MANAGER.UDPSubscribe(PacketType::SEND_ACK, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {

		int criticalPacketId;
		packet.ReadVariable(criticalPacketId, packet.payloadOffset);
		//std::cout << "ACK sent for critical packet ID: " << criticalPacketId << std::endl;
		// Create ACK packet
		CustomUDPPacket ackPacket(UdpPacketType::NORMAL, PacketType::RECEIVE_ACK, packet.playerId);
		ackPacket.WriteVariable(criticalPacketId);
		SendPacketToClient(ackPacket, senderIpAdress, senderPort);
	});

	EVENT_MANAGER.UDPSubscribe(PacketType::RECEIVE_ACK, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		int criticalPacketId = 0;
		packet.ReadVariable(criticalPacketId, packet.payloadOffset);
		std::cout << "ACK received for critical packet ID: " << criticalPacketId << std::endl;
		// Notify CriticalPacketManager that ACK was received
		auto it = inGameClients.find(packet.playerId);

		if (it == inGameClients.end())
		{
			std::cerr << "ACK received from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}

		it->second->OnACKReceived(criticalPacketId);
	});

	EVENT_MANAGER.UDPSubscribe(PacketType::MATCH_FOUND, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		// Create new room
		std::shared_ptr<Room> newRoom = ROOM_MANAGER.CreateRoom();

		// Read data from packet to init the client (ip, port, playerId)
		int playerId;

		for (int i = 0; i < 2; i++)
		{
			packet.ReadVariable(playerId, packet.payloadOffset);

			//Create new Client, add to Room and insert new client into ingameClients map
			std::shared_ptr<Client> player = std::make_shared<Client>(playerId);
			ROOM_MANAGER.JoinRoom(newRoom, player);
			inGameClients.insert({ player->GetId(), player});

			std::cout << "Se ha creado un cliente con la id: " << player->GetId();
		}

		std::cout << inGameClients.size() << std::endl;
	});

	EVENT_MANAGER.UDPSubscribe(PacketType::SEND_POSITION, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received SEND_POSITION packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;

		int movementId;
		float x, y;

		packet.ReadVariable(movementId, packet.payloadOffset);
		packet.ReadVariable(x, packet.payloadOffset);
		packet.ReadVariable(y, packet.payloadOffset);

		client->AddPositionPacket(movementId, x, y);
		});

	EVENT_MANAGER.UDPSubscribe(START_GAME, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);	
		if (it == inGameClients.end())
		{
			std::cerr << "Received START_GAME packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}

		it->second->AddCriticalPacketToSend(packet, senderIpAdress, senderPort);

		 auto roomIt = ROOM_MANAGER.FindRoomById(it->second->GetRoomId());

		 if (roomIt == ROOM_MANAGER.GetRooms().end())
		 {
			 std::cerr << "Room with ID " << it->second->GetRoomId() << " not found." << std::endl;
			 return; // Room not found, ignore the packet
		 }

		 roomIt->get()->Start();
		});

	EVENT_MANAGER.UDPSubscribe(PacketType::SEND_CLIENT_INFORMATION, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {

		auto it = inGameClients.find(packet.playerId);

		if (it == inGameClients.end())
		{
			std::cout << "the client with id: " << packet.playerId << " does not exist" << std::endl;
			return;
		}

		std::shared_ptr<Client> client = inGameClients[packet.playerId];
		client->SetIp(senderIpAdress);
		client->SetPort(senderPort);

		client->AddPlayerReady();
		std::cout << "Saved new network client data" << senderIpAdress << senderPort << std::endl;
		});

	EVENT_MANAGER.UDPSubscribe(SEND_MOCKERY, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received SEND_MOCKERY packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;
		CustomUDPPacket mockeryPacket(UdpPacketType::CRITIC, RECEIVE_MOCKERY, client->GetId());
		
		client->GetOpponentClient()->SendPacketToOpponent(mockeryPacket);
		});

	EVENT_MANAGER.UDPSubscribe(SEND_START_SHOOT, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received SEND_START_SHOOTING packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;

		CustomUDPPacket startShootPacket(UdpPacketType::CRITIC, RECEIVE_START_SHOOT, client->GetId());
		CustomUDPPacket newPacket = client->GetOpponentClient()->AddCriticalPacketToSend(startShootPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
		SendPacketToClient(newPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
		});

	EVENT_MANAGER.UDPSubscribe(SEND_STOP_SHOOT, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received SEND_STOP_SHOOTING packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;
		CustomUDPPacket stopShootPacket(UdpPacketType::CRITIC, RECEIVE_STOP_SHOOT, client->GetId());
		CustomUDPPacket newPacket = client->GetOpponentClient()->AddCriticalPacketToSend(stopShootPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
		SendPacketToClient(newPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
		});

	EVENT_MANAGER.UDPSubscribe(SEND_RESPAWN, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received RESPAWN packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}

		int criticalId, movementId, lives;
		bool value;
		float x, y;

		packet.ReadVariable(criticalId, packet.payloadOffset);
		packet.ReadVariable(value, packet.payloadOffset);
		packet.ReadVariable(movementId, packet.payloadOffset);
		packet.ReadVariable(x, packet.payloadOffset);
		packet.ReadVariable(y, packet.payloadOffset);
		packet.ReadVariable(lives, packet.payloadOffset);

		std::shared_ptr<Client> client = it->second;
		if (value) {
			client->Respawn(movementId, x, y);
		}
		else {
			client->GetOpponentClient()->Respawn(movementId, x, y);
		}

		if (lives == 0) {

			CustomUDPPacket gameOverPacketFirstPlayer(UdpPacketType::NORMAL, END_GAME, client->GetId());
			CustomUDPPacket gameOverPacketSecondPlayer(UdpPacketType::NORMAL, END_GAME, client->GetId());


			if (value) {
				gameOverPacketFirstPlayer.WriteString("You have lost the game, you have no more lives left. Better luck next time!");
				gameOverPacketSecondPlayer.WriteString("You have won the game, your opponent has no more lives left. Congratulations!");


			}
			else {
				gameOverPacketFirstPlayer.WriteString("You have won the game, your opponent has no more lives left. Congratulations!");
				gameOverPacketSecondPlayer.WriteString("You have lost the game, you have no more lives left. Better luck next time!");
			}

			SendPacketToClient(gameOverPacketFirstPlayer, client->GetIp(), client->GetPort());
			SendPacketToClient(gameOverPacketSecondPlayer, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());

			ROOM_MANAGER.DeleteRoom(client->GetRoomId());
		}
		else {
			CustomUDPPacket stopShootPacket(UdpPacketType::CRITIC, RECEIVE_RESPAWN, client->GetId());
			CustomUDPPacket newPacket = client->GetOpponentClient()->AddCriticalPacketToSend(stopShootPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
			newPacket.WriteVariable(value);
			newPacket.WriteVariable(lives);
			std::cout << "las vidas del jugador son:" << criticalId << " " << value << " " << movementId << " " << x << " " << y << " " << lives << std::endl;
			SendPacketToClient(newPacket, client->GetOpponentClient()->GetIp(), client->GetOpponentClient()->GetPort());
		}
	});
	
	EVENT_MANAGER.UDPSubscribe(RECEIVE_PING, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received RECEIVE_PING packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;
		client->OnPongReceived();

	});

	EVENT_MANAGER.UDPSubscribe(SEND_PING, [this](CustomUDPPacket& packet, sf::IpAddress senderIpAdress, int senderPort) {
		auto it = inGameClients.find(packet.playerId);
		if (it == inGameClients.end())
		{
			std::cerr << "Received SEND_PING packet from unknown player ID: " << packet.playerId << std::endl;
			return; // Player not found, ignore the packet
		}
		std::shared_ptr<Client> client = it->second;
		CustomUDPPacket pongPacket(UdpPacketType::NORMAL, RECEIVE_PING, client->GetId());
		SendPacketToClient(pongPacket, client->GetIp(), client->GetPort());
	});
}

void PacketManager::ProcessUDPReceivedPacket(CustomUDPPacket& customPacket, sf::IpAddress senderIpAdress, int senderPort)
{
	auto it = inGameClients.find(customPacket.playerId);


	if (customPacket.udpType == UdpPacketType::CRITIC)
	{
		if (it == inGameClients.end())
		{
			std::cerr << "Received critical packet from unknown player ID: " << customPacket.playerId << std::endl;
			return; // Player not found, ignore the packet
		}

		std::shared_ptr<Client> client = it->second;

		client->AddCriticalPacketIdToSet(customPacket, senderIpAdress, senderPort);
		EVENT_MANAGER.UDPEmit(SEND_ACK, customPacket, senderIpAdress, senderPort);
	}

	if (it != inGameClients.end())
		it->second->OnPongReceived();

	//std::cout << static_cast<int>(customPacket.type) << std::endl;
	EVENT_MANAGER.UDPEmit(customPacket.type, customPacket, senderIpAdress, senderPort);
}

void PacketManager::SendPacketToClient(CustomUDPPacket& responsePacket, sf::IpAddress ipAdress, int port)
{
	if (serverSocket == nullptr)
		throw std::runtime_error("Server socket is not initialized");

	sf::Socket::Status status = serverSocket->send(responsePacket.buffer, responsePacket.bufferSize, ipAdress, port);

	if (status == sf::Socket::Status::Done)
	{
		//std::cout << "Packet sent to " << ipAdress.toString() << ":" << port << std::endl;
	}
	else
	{
		std::cerr << "Failed to send packet to " << ipAdress.toString() << ":" << port << ". Error: " << static_cast<int>(status) << std::endl;
	}
};
