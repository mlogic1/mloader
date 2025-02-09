#ifndef RCLONE_H
#define RCLONE_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	class RClone
	{
		public:
			RClone(const std::string& cacheDir);
			~RClone();

			bool SyncFile(const std::string& baseUrl, const std::string& fileName, const fs::path& directory) const;
			bool CopyFile(const std::string& baseUrl, const std::string& fileId, const fs::path& directory) const;
		
		private:
			bool CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
			fs::path m_rcloneToolPath;	// points to rclone executable
	};
}

#endif // RCLONE_H
