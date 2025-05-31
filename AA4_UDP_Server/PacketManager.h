#pragma once
#include "CustomUDPPacket.h"
#include "Client.h"

#define PACKET_MANAGER PacketManager::Instance()

class PacketManager
{
private:
    PacketManager() = default;
    PacketManager(const PacketManager&) = delete;
    PacketManager& operator=(const PacketManager&) = delete;

    sf::UdpSocket* serverSocket;

public:
    inline static PacketManager& Instance()
    {
        static PacketManager manager;
        return manager;
    }

    void Init(sf::UdpSocket* _serverSocket);

    void SendHandshake(const std::string guid);
    void ProcessUDPReceivedPacket(CustomUDPPacket& customPacket);

    void SendPacketToClient(CustomUDPPacket& responsePacket, sf::IpAddress ipAdress, int port);
};