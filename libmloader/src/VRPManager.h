#ifndef MLOADER_H
#define MLOADER_H

#include "model/GameInfo.h"
#include <mloader/App.h>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <map>

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
			void ChangeGameStatus(const GameInfo& gameInfo, AppStatus newStatus, int statusParam = -1);
			void DownloadGame(const GameInfo& game);
			std::string GetAppThumbImage(const GameInfo& game) const;
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
