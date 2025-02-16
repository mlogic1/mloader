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
		m_7zToolPath = zipToolDir / "7zz";	// Note for windows: i think the tool is called 7z on windowsß∫

	#ifdef _WIN32
		// const std::string httpFile{"https://www.7-zip.org/a/7zr.exe"};
		// const fs::path zipToolPath = zipToolDir / "7z";
		// support for windows is not yet implemented
	#elif __APPLE__
		const std::string httpFile{"https://www.7-zip.org/a/7z2409-mac.tar.xz"};
		const fs::path zipToolPathZip = m_cacheDir / "7z2409-mac.tar.xz";
	#else
		const std::string httpFile{"https://www.7-zip.org/a/7z2409-linux-x64.tar.xz"};
		const fs::path zipToolPathZip = m_cacheDir / "7z2409-linux-x64.tar.xz";
	#endif

		if (!fs::exists(zipToolDir))
		{
			if (!fs::create_directories(zipToolDir))
			{
				throw std::runtime_error("Unable to create directory: " + zipToolDir.string());
			}
		}

		if (!fs::exists(m_7zToolPath))
		{
			if (!fs::exists(zipToolPathZip))
			{
				// log downloading rclone
				if (!CurlDownloadFile(httpFile, zipToolPathZip))
				{
					throw std::runtime_error("Unable to download 7z");
				}
			}

			// unzip
			FILE* fp;
			char strbuffer[512];
			snprintf(strbuffer, sizeof(strbuffer), "tar -xvJf %s -C %s 7zz", zipToolPathZip.c_str(), zipToolDir.c_str());
			const std::string dbgStr = strbuffer;
			fp = popen(strbuffer, "r");
			if (fp == NULL)
			{
				perror("popen");
				return false;
			}
			
			char path[1035];
			// Read the output a line at a time - output it.
			while (fgets(path, sizeof(path), fp) != NULL) {
				// printf("%s", path);
			}

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

	bool Zip::Unzip7z(const fs::path& archiveFile, const fs::path& destinationDir, const std::string& password) const
	{
		if (!fs::exists(archiveFile))
		{
			return false;
		}

		// unzip
		FILE* fp;
		char strbuffer[512];
		snprintf(strbuffer, sizeof(strbuffer), "%s x -aoa -bsp2 -o%s -p%s %s 2>&1", m_7zToolPath.c_str(), destinationDir.c_str(), password.c_str(), archiveFile.c_str());
		//  -bso0
		const std::string dbgStr = strbuffer;
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			return false;
		}

		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) { }

		int status = pclose(fp);
		if (status == -1) {
			perror("pclose");
			return false;	// TODO: report this through a callback
		} else {
			// printf("7zz Command exited with status: %d\n", WEXITSTATUS(status));
		}

		return true;
	}
}
