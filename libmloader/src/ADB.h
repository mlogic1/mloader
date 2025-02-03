#ifndef ADB_H
#define ADB_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	class ADB
	{
		public:
			ADB(const std::string& cacheDir);
			~ADB();
		
		private:
			bool CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
	};
}

#endif // ADB_H
