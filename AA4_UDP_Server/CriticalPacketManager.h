#pragma once
#include <unordered_map>
#include "CustomUDPPacket.h"
#include <mutex>

struct CriticalPacket
{
	CustomUDPPacket packet;
	int criticalPacketId;
	int resendAttempts = 0;
	float resendDelay = 0.5f; // seconds
	float timeSinceLastSend = 0.0f;

	sf::IpAddress targetIp;
	unsigned short targetPort;

	CriticalPacket() = default;

	CriticalPacket(CustomUDPPacket& packet, int criticalPacketId, sf::IpAddress targetAdress, unsigned short targetPort)
		: packet(packet), criticalPacketId(criticalPacketId), targetIp(targetAdress), targetPort(targetPort) {
	}
};

#define CRITICAL_PACKET_MANAGER CriticalPacketManager::Instance()

class CriticalPacketManager
{
private:

	 

	int packetCounter = 0;
	std::mutex packetMutex;

	CriticalPacketManager() = default;
	CriticalPacketManager(const CriticalPacketManager&) = delete;
	CriticalPacketManager& operator=(const CriticalPacketManager&) = delete;

public:

	static CriticalPacketManager& Instance()
	{
		static CriticalPacketManager instance;
		return instance;
	}

	void StartCriticalPacketUpdateThread();

};



