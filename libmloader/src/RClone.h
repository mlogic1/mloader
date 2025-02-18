#ifndef RCLONE_H
#define RCLONE_H

#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace mloader
{
	class Logger;
	class RClone
	{
		public:
			RClone(const std::string& cacheDir, Logger& logger);
			~RClone();

			bool SyncFile(const std::string& baseUrl, const std::string& fileName, const fs::path& directory) const;
			bool CopyFile(const std::string& baseUrl, const std::string& fileId, const fs::path& directory, std::function<void(uint8_t)> progressCallback = nullptr) const;
		
		private:
			bool CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
			fs::path m_rcloneToolPath;	// points to rclone executable

			Logger& m_logger;
			static constexpr const char* LOG_NAME = "RClone";
	};
}

#endif // RCLONE_H
