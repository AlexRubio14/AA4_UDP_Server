#include "Client.h"
#include "RoomManager.h"
#include "CustomUDPPacket.h"
#include "PacketManager.h"

Client::Client(int playerId) : 
	playerId(playerId),
	position(0.f, 0.f),
	health(5),
	lives(3),
	isAlive(true),
	roomId(-1),
	ipAddress(sf::IpAddress::Any),
	isReadyToPlay(false)
{}

void Client::AddPlayerReady()
{
	isReadyToPlay = true;
	ROOM_MANAGER.FindRoomById(roomId)->get()->CheckIfAllPlayersReady();
}

void Client::AddCriticalPacketIdToSet(CustomUDPPacket& packet, sf::IpAddress targetAdress, unsigned short port)
{
	std::lock_guard<std::mutex> lock(packetMutex);
	int id;
	id = packet.ReadVariable(id, packet.payloadOffset); // Read the packet ID from the packet
	if (criticalPacketsIdReceived.find(id) != criticalPacketsIdReceived.end())
	{
		std::cerr << "Critical packet with ID " << id << " already exists in received set." << std::endl;
		return; // Packet with this ID already exists, do not add again
	}

	criticalPacketsIdReceived.insert(id); // Add the ID to the set of received critical packets
}

void Client::AddPacketToSend(const CustomUDPPacket& packet, sf::IpAddress targetAdress, unsigned short port)
{
	std::lock_guard<std::mutex> lock(packetMutex);
	int id = packetCounter++;

	CustomUDPPacket packetCopy = packet; // Create a copy of the packet

	pendingPacketsToSend.emplace(id, CriticalPacket(packetCopy, id, targetAdress, port));

	PACKET_MANAGER.SendPacketToClient(packetCopy, targetAdress, port);
}

void Client::AddPositionPacket(int movementId, int x, int y)
{
	positionPackets.emplace_back(movementId, x, y);
}

void Client::CriticalPacketsUpdate(float deltaTime)
{
	std::lock_guard<std::mutex> lock(packetMutex);
	for (auto& [id, criticalPacket] : pendingPacketsToSend)
	{
		criticalPacket.timeSinceLastSend += deltaTime;
		if (criticalPacket.timeSinceLastSend >= criticalPacket.resendDelay)
		{
			if (criticalPacket.resendAttempts < 8) // Max 3 resend attempts
			{
				PACKET_MANAGER.SendPacketToClient(criticalPacket.packet, criticalPacket.targetIp, criticalPacket.targetPort);
				criticalPacket.resendAttempts++;
				criticalPacket.timeSinceLastSend = 0.0f;
				criticalPacket.resendDelay = std::min(criticalPacket.resendDelay * 2.0f, 5.0f); // Exponential backoff, max 5 seconds
			}
			else
			{
				//TODO: desconectar al cliente si no se recibe ACK después de 3 intentos
				pendingPacketsToSend.erase(id); // Remove packet after max attempts
			}
		}
	}
}

void Client::OnACKReceived(int packetId)
{
	std::lock_guard<std::mutex> lock(packetMutex);
	pendingPacketsToSend.erase(packetId); // Remove the packet from pending packets on ACK

	std::cout << "ACK received from client with id :" << packetId << " erased " << std::endl;
}

void Client::ValidateClientMovements(int playerId)
{
	// std::cout << "[Server] Validating movements for player " << playerId << std::endl;
	if (positionPackets.empty())
		return;

	// std::cout << "[Server] Received " << positionPackets.size() << " position packets for player " << playerId << std::endl;

	std::vector<PositionPacket> validPackets;

	PositionPacket prev = positionPackets[0];
	validPackets.push_back(prev);

	for (size_t i = 1; i < positionPackets.size(); ++i)
	{
		const PositionPacket& current = positionPackets[i];

		// Ignorar paquetes antiguos fuera de orden
		if (current.movementId <= prev.movementId)
			continue;

		int deltaSteps = current.movementId - prev.movementId;
		float deltaTime = deltaSteps * TIME_PER_PACKET;

		float dx = static_cast<float>(current.x - prev.x);
		float dy = static_cast<float>(current.y - prev.y);

		float speedX = std::abs(dx / deltaTime);
		float speedY = std::abs(dy / deltaTime);

		if (speedX > MAX_SPEED_X + TOLERANCE || speedY > MAX_SPEED_Y + TOLERANCE)
		{

			// Send to player a correction packet with the last valid position
			CustomUDPPacket correction(UdpPacketType::NORMAL, VALIDATION_BACK, playerId);
			correction.WriteVariable(prev.movementId);
			PACKET_MANAGER.SendPacketToClient(correction, ipAddress, port);

			std::cout << "[Server] Invalid movement detected for player " << playerId
				<< " between IDs " << prev.movementId << " and " << current.movementId
				<< " (speedX=" << speedX << ", speedY=" << speedY << ")" << std::endl;

			// Send to player the positions of opponent
			for (const PositionPacket& packet : validPackets)
			{
				CustomUDPPacket interpolationPacket(UdpPacketType::NORMAL, INTERPOLATION_POSITION, playerId);
				interpolationPacket.WriteVariable(packet.movementId);
				interpolationPacket.WriteVariable(packet.x);
				interpolationPacket.WriteVariable(packet.y);
				PACKET_MANAGER.SendPacketToClient(interpolationPacket, opponentClient->GetIp(), opponentClient->GetPort());
				std::cout << "[Server] Sending interpolation packet for player " << playerId
					<< " with movement ID " << packet.movementId
					<< " to opponent at " << opponentClient->GetIp() << ":" << opponentClient->GetPort() << std::endl;
			}

			positionPackets.clear();
			positionPackets.push_back(prev);
			return;
		}

		validPackets.push_back(current);
		prev = current;
	}

	CustomUDPPacket validated(UdpPacketType::NORMAL, VALIDATION_OK, playerId);
	PACKET_MANAGER.SendPacketToClient(validated, ipAddress, port);

	std::cout << "ALL POSITIONS VALID" << std::endl;

	for (const PositionPacket& packet : validPackets)
	{
		CustomUDPPacket interpolationPacket(UdpPacketType::NORMAL, INTERPOLATION_POSITION, playerId);
		interpolationPacket.WriteVariable(packet.movementId);
		interpolationPacket.WriteVariable(packet.x);
		interpolationPacket.WriteVariable(packet.y);
		PACKET_MANAGER.SendPacketToClient(interpolationPacket, opponentClient->GetIp(), opponentClient->GetPort());
		std::cout << "[Server] Sending interpolation packet for player " << playerId
			<< " with movement ID " << packet.movementId
			<< " to opponent at " << opponentClient->GetIp() << ":" << opponentClient->GetPort() << std::endl;
	}
	

	// mantener solo la última como base
	PositionPacket last = validPackets.back();
	positionPackets.clear();
	positionPackets.push_back(last);
}
