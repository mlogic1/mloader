#ifndef SEVENZ_H
#define SEVENZ_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	class Zip
	{
		public:
			Zip(const std::string& cacheDir);
			~Zip();

			bool Unzip7z(const fs::path& archiveFile, const fs::path& destinationDir, const std::string& password = "") const;

		private:
			bool CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
			fs::path m_7zToolPath;	// points to 7z executable
	};
}

#endif // SEVENZ_H
