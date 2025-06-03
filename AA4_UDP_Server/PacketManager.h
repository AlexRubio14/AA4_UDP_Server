#pragma once
#include "CustomUDPPacket.h"
#include "Client.h"
#include <unordered_map>
#include <memory>

#define PACKET_MANAGER PacketManager::Instance()

class PacketManager
{
private:
    PacketManager() = default;
    PacketManager(const PacketManager&) = delete;
    PacketManager& operator=(const PacketManager&) = delete;

    sf::UdpSocket* serverSocket;
    std::unordered_map<int, std::shared_ptr<Client>> inGameClients = {};

public:
    inline static PacketManager& Instance()
    {
        static PacketManager manager;
        return manager;
    }

    void Init(sf::UdpSocket* _serverSocket);

    void SendHandshake(const std::string guid);
    void ProcessUDPReceivedPacket(CustomUDPPacket& customPacket, sf::IpAddress senderIpAdress, int senderPort);

    void SendPacketToClient(CustomUDPPacket& responsePacket, sf::IpAddress ipAdress, int port);

	std::unordered_map<int, std::shared_ptr<Client>> GetInGameClients() const { return inGameClients; }
};