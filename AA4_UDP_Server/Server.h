#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "Client.h"
#define LISTENER_PORT 55002

class Server {
private:
    std::unique_ptr<sf::UdpSocket> socket;
	char serverBuffer[1024];
	std::size_t receivedBufferSize;

    bool isRunning;

public:

    Server();

    ~Server();

    void Start();

    void Update();

    void ClearBuffer();
};