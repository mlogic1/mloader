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

		private:
			bool CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
	};
}

#endif // SEVENZ_H