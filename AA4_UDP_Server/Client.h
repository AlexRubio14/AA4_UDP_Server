#pragma once
#include "SFML/Network.hpp"

struct ClientData {
	sf::IpAddress ipAddress;
	int port;
	int playerId;
};

class Client
{
private: 

	sf::Vector2f position;
	int health;
	int lives;
	bool isAlive;

	std::chrono::steady_clock::time_point lastInputTime;

	//Network
	sf::IpAddress ipAddress;
	int port;
};
