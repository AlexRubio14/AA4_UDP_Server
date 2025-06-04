#pragma once
#include "SFML/Network.hpp"
#include <chrono>
#include <memory>
#include <unordered_map>
#include "CriticalPacketManager.h"
#include <unordered_set>
#include <vector>

struct PositionPacket
{
	int movementId;
	float x;
	float y;

	PositionPacket() = default;
	PositionPacket(int id, float x, float y) : movementId(id), x(x), y(y) {}
};

class Client
{
private: 

	const float MAX_SPEED_X = 150.f;
	const float MAX_SPEED_Y = 500.f;   // depende de tu jumpForce y gravedad //Tolerancia para proabr con alex 280
	const float TOLERANCE = 5.f;
	const float TIME_PER_PACKET = 0.05f;

	sf::Vector2f position;
	int health;
	int lives;
	bool isAlive;

	bool isReadyToPlay;

	std::chrono::steady_clock::time_point lastInputTime;

	int roomId;

	//Network data
	sf::IpAddress ipAddress;
	int port;
	int playerId;

	std::unordered_set<int> criticalPacketsIdReceived;
	std::unordered_map<int, CriticalPacket> pendingPacketsToSend;

	std::vector<PositionPacket> positionPackets = {};

	std::mutex packetMutex;
	std::mutex positionMutex;
	int packetCounter = 0;

	std::shared_ptr<Client> opponentClient;

public:

	Client(int playerId);

	void AddPlayerReady();

	void AddCriticalPacketIdToSet(const CustomUDPPacket& packet, sf::IpAddress targetAdress, unsigned short port);
	CustomUDPPacket AddCriticalPacketToSend(const CustomUDPPacket& packet, sf::IpAddress targetAdress, unsigned short port);

	void AddPositionPacket(int movementId, int x, int y);

	void CriticalPacketsUpdate(float deltaTime);

	void OnACKReceived(int packetId);

	void SetOpponentClient(const std::shared_ptr<Client> opponent) { opponentClient = opponent; }

	void ValidateClientMovements(int playerId);

	void SendPacketToOpponent(CustomUDPPacket& packet);

	void Respawn(int movementId, float x, float y);

	inline int GetRoomId() const { return roomId; }
	inline sf::IpAddress GetIp() const { return ipAddress; }
	inline int GetPort() const { return port; }
	inline bool GetIsreadyToPlay() const { return isReadyToPlay; }
	inline int GetId() const { return playerId; }
	inline std::vector<PositionPacket> GetPositionPackets() const { return positionPackets; }
	inline std::shared_ptr<Client> GetOpponentClient() const { return opponentClient; }

	inline void SetIp(const sf::IpAddress& ip) { this->ipAddress = ip; }
	inline void SetPort(const int port) { this->port = port; }
	inline void SetRoomId(const int roomId) { this->roomId = roomId; }
};
