#ifndef MLOADER_H
#define MLOADER_H

#include "model/GameInfo.h"
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace mloader
{
	class RClone;
	class Zip;

	class VRPManager
	{
		public:
			VRPManager(const RClone& rclone, const Zip& zip, const fs::path& cacheDir, const fs::path& downloadDir);
			~VRPManager();

			bool RefreshMetadata(bool forceRedownload = false);

			const std::vector<GameInfo>& GetGameList() const;
			void DownloadGame(const GameInfo& game) const;

		private:
			bool CheckVRPPublicCredentials();
			bool DownloadMetadata();
			
		private:
			const RClone& m_rClone;
			const Zip& m_zip;
			fs::path m_cacheDir;
			fs::path m_downloadDir;

			std::vector<GameInfo> m_gameList;

			std::string m_baseUri = "https://go.vrpyourself.online/";
			std::string m_password = "gL59VfgPxoHR";
	};
}

#endif // MLOADER_H