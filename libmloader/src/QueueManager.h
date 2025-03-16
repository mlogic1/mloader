// Copyright (c) 2025 mlogic1
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include "atomic"
#include "model/GameInfo.h"
#include "ADB.h"
#include "Logger.h"
#include "VRPManager.h"
#include <mutex>
#include <thread>
#include <queue>

namespace mloader
{
	class QueueManager
	{
		public:
			QueueManager(VRPManager& vrpManager, ADB& adb, Logger& logger);
			~QueueManager();

			void QueueDownload(const GameInfo* game);
			void QueueInstall(const GameInfo* game);

			void SetSelectedAdbDevice(AdbDevice* device);

			void ClearDownloadQueue();
			void ClearInstallQueue();

		private:
			void BackgroundDownloadService();
			void BackgroundInstallService();

		private:
			std::atomic_bool m_running;
			std::mutex m_downloadQueueMutex;
			std::mutex m_installQueueMutex;
			std::queue<const GameInfo*> m_installQueue;
			std::queue<const GameInfo*> m_downloadQueue;

		private:
			VRPManager& m_vrpManager;
			ADB&		m_adb;
			Logger&		m_logger;

			AdbDevice* m_selectedDevice = nullptr;

			static constexpr const char* LOG_NAME{"QueueManager"};
	};
}

#endif // QUEUE_MANAGER_H
