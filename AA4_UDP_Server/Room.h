#pragma once
#include <vector>
#include "Client.h"
#include <memory>
#include <iostream>
#include <mutex>

class Room
{
private:

	const sf::Time interval = sf::seconds(0.1f);

	int id;
	std::vector<std::shared_ptr<Client>> clients;

	sf::Clock positionPacketClock;

	bool running;

	std::mutex roomMutex;
public:

	Room() = default;
	Room(const int id);
	~Room();

	void AddClient(std::shared_ptr<Client> client);
	void RemoveClient(std::shared_ptr<Client> client);
	void RemoveClient(int playerId);

	void CheckIfAllPlayersReady();

	void Start();
	void Update();

	void FinishRoom();


	inline int GetId() const { return id; }
	inline void SetId(const int id) { this->id = id; }
	inline std::vector<std::shared_ptr<Client>> GetClients() const { return clients; }
};