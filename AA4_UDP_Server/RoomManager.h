#pragma once
#include <vector>
#include <memory>
#include "Room.h"
#include "Client.h"

#define ROOM_MANAGER RoomManager::Instance()

class RoomManager
{
private:
	RoomManager();
	RoomManager(const RoomManager&) = delete;
	RoomManager& operator=(const RoomManager&) = delete;

	std::vector<std::shared_ptr<Room>> rooms;
	int roomsCount = 0;
public:
	static RoomManager& Instance();
	std::shared_ptr<Room> CreateRoom();
	void JoinRoom(std::shared_ptr<Room> room, std::shared_ptr<Client> client);
	void LeaveRoom(std::shared_ptr<Client> client);
	void DeleteRoom(const int roomId);
	std::vector<std::shared_ptr<Room>>::iterator FindRoomById(const int roomId);

	inline std::vector<std::shared_ptr<Room>> GetRooms() const { return rooms; }
};