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
		
		private:
			bool CheckAndDownloadTool();
			
			// bool copyFile

		private:
			fs::path m_cacheDir;
	};
}

#endif // RCLONE_H