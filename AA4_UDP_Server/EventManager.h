#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "PacketType.h"
#include "PacketManager.h"

#define EVENT_MANAGER EventManager::Instance()

class EventManager
{
public:
    using UDPCallback = std::function<void(CustomUDPPacket&, sf::IpAddress, int)>;


    void UDPSubscribe(const PacketType type, UDPCallback callback);

    void UDPEmit(const PacketType type, CustomUDPPacket customPacket, sf::IpAddress senderIpAdress, int senderPort);

    static EventManager& Instance();

private:

    std::unordered_map<PacketType, std::vector<UDPCallback>> UDPsubscribers;

    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
};

