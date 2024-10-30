#include "7z.h"
#include "curl_global.h"
#include <filesystem>
#include <exception>
#include <iostream>
#include <fstream>

namespace mloader
{
	Zip::Zip(const std::string& cacheDir)
		: m_cacheDir(cacheDir)
	{
		CheckAndDownloadTool();
	}

	Zip::~Zip()
	{

	}

	bool Zip::CheckAndDownloadTool()
	{

		const fs::path zipToolDir = m_cacheDir / "7z/";

	#ifdef _WIN32
		const std::string httpFile{"https://www.7-zip.org/a/7zr.exe"};
		const fs::path zipToolPath = zipToolDir / "7z";
		const fs::path zipToolPathZip = m_cacheDir / "rclone-v1.69.0-linux-amd64.zip";
	#else
		const std::string httpFile{"https://www.7-zip.org/a/7z2409-linux-x64.tar.xz"};
		
		const fs::path zipToolPath = zipToolDir / "7z";
		const fs::path zipToolPathZip = m_cacheDir / "rclone-v1.69.0-linux-amd64.zip";
	#endif

		if (!fs::exists(zipToolPath))
		{
			if (!fs::exists(zipToolPathZip))
			{
				// log downloading rclone
				if (!CurlDownloadFile(httpFile, zipToolPathZip))
				{
					throw std::runtime_error("Unable to download 7z");
				}
			}

			// extract
			// TODO: format this string
			FILE* fp;
			// fp = popen("7z x -o/tmp/mloader/cache/ /tmp/mloader/cache/rclone-v1.69.0-linux-amd64.zip", "r");
			fp = popen("unzip /tmp/mloader/cache/rclone-v1.69.0-linux-amd64.zip -d /tmp/mloader/cache/", "r");
			if (fp == NULL)
			{
				perror("popen");
				return false;
			}
			
			/*char path[1035];
			// Read the output a line at a time - output it.
			while (fgets(path, sizeof(path), fp) != NULL) {
				printf("%s", path);
			}*/

			int status = pclose(fp);
			if (status == -1) {
				perror("pclose");
				return false;
			} else {
				printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}

		return true;
	}
}
