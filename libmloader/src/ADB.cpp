#include "ADB.h"
#include "curl_global.h"
#include <filesystem>
#include <exception>
#include <iostream>
#include <fstream>

namespace mloader
{
	ADB::ADB(const std::string& cacheDir)
		: m_cacheDir(cacheDir)
	{
		CheckAndDownloadTool();
	}

	ADB::~ADB()
	{

	}

	bool ADB::CheckAndDownloadTool()
	{
		const fs::path adbToolDir = m_cacheDir / "platform-tools/";
		m_adbToolPath = adbToolDir/ "adb";
	#ifdef _WIN32
		// const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-windows.zip"};
		// no support for windows yet
	#elif __APPLE__
		// x86 Intel (no support for apple arm yet.)
		const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-darwin.zip"};
		const fs::path adbToolPathZip = m_cacheDir / "platform-tools-latest-darwin.zip";
		
	#else
		const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-linux.zip"};
		const fs::path adbToolPathZip = m_cacheDir / "platform-tools-latest-linux.zip";
	#endif

		if (!fs::exists(m_adbToolPath))
		{
			if (!fs::exists(adbToolPathZip))
			{
				// log downloading adb
				if (!CurlDownloadFile(httpFile, adbToolPathZip))
				{
					throw std::runtime_error("Unable to download ADB");
				}
			}

			// extract
			// TODO: format this string
			FILE* fp;
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), "unzip %s -d %s", adbToolPathZip.c_str(), m_cacheDir.c_str());
			const std::string dbgStr = buffer;
			fp = popen(buffer, "r");
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
			} 
			else
			{
				// printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}

		return true;
	}
}
