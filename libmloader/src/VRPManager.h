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

#ifndef MLOADER_H
#define MLOADER_H

#include "model/GameInfo.h"
#include <mloader/VrpApp.h>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace mloader
{
	class RClone;
	class Zip;
	class Logger;

	class VRPManager
	{
		public:
			VRPManager(const RClone& rclone, const Zip& zip, const fs::path& cacheDir, const fs::path& downloadDir, Logger& logger, std::function<void(const GameInfo&, const AppStatus, const int)> gameStatusChangedCallback = nullptr);
			~VRPManager();

			bool RefreshMetadata(bool forceRedownload = false);

			const std::map<GameInfo, AppStatus>& GetGameList() const;
			AppStatus GetGameStatus(const GameInfo& gameInfo) const;
			void UpdateGameStatus(const GameInfo& gameInfo, AppStatus newStatus, int statusParam = -1);
			void DownloadGame(const GameInfo& game);
			void DeleteGame(const GameInfo& game);
			std::string GetAppThumbImage(const GameInfo& game) const;
			std::string GetAppNote(const GameInfo& game) const;
			bool GameInstalled(const GameInfo& game) const;
			std::vector<fs::path> GetGameFileList(const GameInfo& game) const;

		private:
			bool CheckVRPPublicCredentials();
			bool DownloadMetadata();

		private:
			const RClone& m_rClone;
			const Zip& m_zip;
			fs::path m_cacheDir;
			fs::path m_downloadDir;

			std::map<GameInfo, AppStatus> m_gameList;
			std::function<void(const GameInfo&, const AppStatus, const int)> m_gameStatusChangedCallback = nullptr;

			std::string m_baseUri = "https://go.vrpyourself.online/";
			std::string m_password = "gL59VfgPxoHR";

			Logger& m_logger;
			static constexpr const char* LOG_NAME = "VRPManager";
	};
}

#endif // MLOADER_H
