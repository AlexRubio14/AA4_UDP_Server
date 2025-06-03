#include "Room.h"
#include "EventManager.h"
#include "RoomManager.h"
#include "CriticalPacketManager.h"


Room::Room(const int id)
{
	this->id = id;
}

Room::~Room()
{
	std::cout << "Room erased" << std::endl;
	running = false;
}

void Room::AddClient(std::shared_ptr<Client> client)
{
	clients.push_back(client);
	std::cout << "Client added to room" << std::endl;
}

void Room::RemoveClient(std::shared_ptr<Client> client)
{
	auto clientsIt = std::remove(clients.begin(), clients.end(), client);
	if (clientsIt != clients.end())
	{
		clients.erase(clientsIt, clients.end());
		//std::cout << "Client: " << client->ge << " removed from room" << std::endl;
	}
	else
		std::cerr << "Client not found in room" << std::endl;
}

void Room::CheckIfAllPlayersReady()
{
	for (std::shared_ptr<Client> client : clients)
	{
		if (!client->GetIsreadyToPlay())
		{
			std::cout << "Not all players are ready" << std::endl;
			return;
		}
	}

	clients[0]->SetOpponentClient(clients[1]);
	clients[1]->SetOpponentClient(clients[0]);

	for (std::shared_ptr<Client> client : clients)
	{
		EVENT_MANAGER.UDPEmit(PacketType::START_GAME, CustomUDPPacket(UdpPacketType::CRITIC, START_GAME, client->GetId()), client->GetIp(), client->GetPort());
	}
}

void Room::Start()
{
	running = true;

	std::thread([this]() {
		constexpr float UPDATE_RATE = 1.0f / 30.0f; // 30 updates por segundo
		while (running)
		{
			auto start = std::chrono::high_resolution_clock::now();

			Update(); // aquí actualizas físicas, colisiones, validación...

			auto end = std::chrono::high_resolution_clock::now();
			float elapsed = std::chrono::duration<float>(end - start).count();
			float sleepTime = UPDATE_RATE - elapsed;
			if (sleepTime > 0)
				std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
		}
		}).detach();
}

void Room::Update()
{
	if (positionPacketClock.getElapsedTime() >= interval)
	{
		positionPacketClock.restart();
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i])
				clients[i]->ValidateClientMovements(i);
		}
	}
}
