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

#include "QueueManager.h"
#include <algorithm>

namespace mloader
{
	QueueManager::QueueManager(VRPManager& vrpManager, ADB& adb, Logger& logger) :
		m_vrpManager(vrpManager),
		m_adb(adb),
		m_logger(logger),
		m_running(true)
	{
		std::thread backgroundDownloadThread(&QueueManager::BackgroundDownloadService, this);
		backgroundDownloadThread.detach();

		std::thread backgroundInstallThread(&QueueManager::BackgroundInstallService, this);
		backgroundInstallThread.detach();
	}

	QueueManager::~QueueManager()
	{
		ClearDownloadQueue();
		ClearInstallQueue();
		m_running = false;
		m_selectedDevice = nullptr;
	}

	void QueueManager::QueueDownload(const GameInfo* game)
	{
		{
			std::lock_guard<std::mutex> lock(m_downloadQueueMutex);
			m_downloadQueue.push(game);
		}
		m_vrpManager.UpdateGameStatus(*game, AppStatus::DownloadQueued);
	}

	void QueueManager::QueueInstall(const GameInfo* game)
	{
		{
			std::lock_guard<std::mutex> lock(m_installQueueMutex);
			m_installQueue.push(game);
		}
		m_vrpManager.UpdateGameStatus(*game, AppStatus::InstallQueued);
	}

	void QueueManager::SetSelectedAdbDevice(AdbDevice* device)
	{
		std::lock_guard<std::mutex> lock(m_installQueueMutex);
		m_selectedDevice = device;

		std::vector<std::string> installedPackages;
		if(m_selectedDevice != nullptr)
		{
			installedPackages = m_adb.GetDeviceThirdPartyPackages(*device);
		}

		for (auto game : m_vrpManager.GetGameList())
		{
			if (game.second >= AppStatus::Downloaded)
			{
				if (std::count(installedPackages.cbegin(), installedPackages.cend(), game.first.PackageName) > 0)
				{
					m_vrpManager.UpdateGameStatus(game.first, AppStatus::Installed);
				}
				else
				{
					m_vrpManager.UpdateGameStatus(game.first, AppStatus::Downloaded);
				}
			}
		}
	}

	void QueueManager::ClearDownloadQueue()
	{
		std::lock_guard<std::mutex> lock(m_downloadQueueMutex);
		while(!m_downloadQueue.empty())
		{
			m_downloadQueue.pop();
		}
	}

	void QueueManager::ClearInstallQueue()
	{
		std::lock_guard<std::mutex> lock(m_installQueueMutex);
		while(!m_installQueue.empty())
		{
			m_installQueue.pop();
		}

		for (auto game : m_vrpManager.GetGameList())
		{
			if (game.second >= AppStatus::Installing)
			{
				m_vrpManager.UpdateGameStatus(game.first, AppStatus::Downloaded);
			}
		}
	}

	void QueueManager::BackgroundDownloadService()
	{
		m_logger.LogInfo(LOG_NAME, "Started background download service");
		while(m_running)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// maybe it would be good to check m_running again here, on rare occasions it crashes when trying to call lock when the application is exiting (or alternatively try sleep at the end of the loop)

			m_downloadQueueMutex.lock();
			if (!m_downloadQueue.empty())
			{
				const GameInfo* gameInfo = m_downloadQueue.back();
				AppStatus gameStatus = m_vrpManager.GetGameStatus(*gameInfo);

				if (gameStatus == AppStatus::DownloadQueued)
				{
					m_downloadQueue.pop();
					m_downloadQueueMutex.unlock();
					m_vrpManager.DownloadGame(*gameInfo);
				}
			}
			m_downloadQueueMutex.unlock();
		}
	}

	void QueueManager::BackgroundInstallService()
	{
		m_logger.LogInfo(LOG_NAME, "Started background install service");
		while(m_running)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// maybe it would be good to check m_running again here, on rare occasions it crashes when trying to call lock when the application is exiting (or alternatively try sleep at the end of the loop)

			if (m_selectedDevice == nullptr)
			{
				continue;
			}

			m_installQueueMutex.lock();
			if (!m_installQueue.empty())
			{
				const GameInfo* gameInfo = m_installQueue.back();
				AppStatus gameStatus = m_vrpManager.GetGameStatus(*gameInfo);
				m_logger.LogInfo(LOG_NAME, "checking status");
				if (gameStatus == AppStatus::InstallQueued)
				{
					m_logger.LogInfo(LOG_NAME, "status check pass");
					m_installQueue.pop();
					m_installQueueMutex.unlock();
					try
					{
						m_logger.LogInfo(LOG_NAME, "setting to installing");
						m_vrpManager.UpdateGameStatus(*gameInfo, AppStatus::Installing);
						std::vector<fs::path> fileList = m_vrpManager.GetGameFileList(*gameInfo);
						m_adb.InstallFilesToDevice(gameInfo->PackageName, fileList, *m_selectedDevice);
						m_vrpManager.UpdateGameStatus(*gameInfo, AppStatus::Installed);
					}
					catch(std::runtime_error& err)
					{
						m_vrpManager.UpdateGameStatus(*gameInfo, AppStatus::InstallingError);
						m_logger.LogError(LOG_NAME, err.what());
					}
				}
			}
			m_installQueueMutex.unlock();
		}
	}
}
