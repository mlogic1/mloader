#ifndef MLOADER_H
#define MLOADER_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	class RClone;

	class VRPManager
	{
		public:
			VRPManager(const RClone& rclone, const fs::path& cacheDir, const fs::path& downloadDir);
			~VRPManager();


			bool DbgRefreshMeta();
			bool RefreshMeta();

		private:
			bool CheckVRPPublicCredentials();
			
		private:
			const RClone& m_rClone;
			fs::path m_cacheDir;
			fs::path m_downloadDir;
	};
}

#endif // MLOADER_H