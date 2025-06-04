#include "CriticalPacketManager.h"
#include "PacketManager.h"
#include <iostream>
void CriticalPacketManager::StartCriticalPacketUpdateThread()
{
	std::thread([]() {
		float updateRateSeconds = 0.05f;

		while (true)
		{
			auto start = std::chrono::high_resolution_clock::now();

			for (auto& [id, client] : PACKET_MANAGER.GetInGameClients())
			{
				client->CriticalPacketsUpdate(updateRateSeconds);
			}

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			float sleepTime = updateRateSeconds - elapsed;

			if (sleepTime > 0)
			{
				std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
			}
		}
	}).detach();
}
