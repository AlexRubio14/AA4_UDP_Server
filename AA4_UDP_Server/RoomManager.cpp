#include "RoomManager.h"
#include <iostream>

RoomManager::RoomManager() {}

RoomManager& RoomManager::Instance()
{
	static RoomManager instance;
	return instance;
}

std::shared_ptr<Room> RoomManager::CreateRoom()
{
	std::shared_ptr<Room> room = std::make_shared<Room>(++roomsCount);
	rooms.push_back(room);
	return room;
}

void RoomManager::JoinRoom(std::shared_ptr<Room> room, std::shared_ptr<Client> client)
{
	room->AddClient(client);
	client->SetRoomId(room->GetId());
}

void RoomManager::LeaveRoom(int roomId,int playerId)
{
	auto roomIt = FindRoomById(roomId);

	if (roomIt != rooms.end())
	{
		roomIt->get()->RemoveClient(playerId);
	}
	else
	{
		std::cerr << "Room with ID " << playerId << " not found." << std::endl;
	}
}

void RoomManager::DeleteRoom(const int roomId)
{
	auto roomIt = FindRoomById(roomId);

	if (roomIt != rooms.end())
	{
		rooms.erase(roomIt);
		std::cout << "Room with ID " << roomId << " deleted." << std::endl;
	}
	else
	{
		std::cerr << "Room with ID " << roomId << " not found." << std::endl;
	}
}

std::vector<std::shared_ptr<Room>>::iterator RoomManager::FindRoomById(const int roomId)
{
	auto roomIt = std::find_if(rooms.begin(), rooms.end(),
		[roomId](const std::shared_ptr<Room>& room) {
			return room->GetId() == roomId;
		});

	if (roomIt != rooms.end())
		std::cout << "Room with ID " << roomId << " found." << std::endl;
	else
		std::cerr << "Room with ID " << roomId << " not found." << std::endl;

	return roomIt;
}