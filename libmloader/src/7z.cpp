// Copyright (c) 2025 mlogic1
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "7z.h"
#include "Logger.h"
#include "curl_global.h"
#include <filesystem>
#include <exception>
#include <cstring>
#include <iostream>
#include <fstream>

namespace mloader
{
	Zip::Zip(const std::string& cacheDir, Logger& logger)
		: m_cacheDir(cacheDir),
		  m_logger(logger)
	{
		CheckAndDownloadTool();
	}

	Zip::~Zip() { }

	bool Zip::CheckAndDownloadTool()
	{
		const fs::path zipToolDir = m_cacheDir / "7z/";
		m_7zToolPath = zipToolDir / "7zz";

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
			if (!fs::create_directory(zipToolDir))
			{
				throw std::runtime_error("Unable to create directory: " + zipToolDir.string());
			}
		}

		if (!fs::exists(m_7zToolPath))
		{
			if (!fs::exists(zipToolPathZip))
			{
				m_logger.LogInfo(LOG_NAME, "Downloading 7z");
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
				m_logger.LogInfo(LOG_NAME, "Unzipping 7z failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
				return false;
			}

			char path[1035];
			// Read the output a line at a time - output it.
			while (fgets(path, sizeof(path), fp) != NULL) {
				// printf("%s", path);
			}

			int status = pclose(fp);
			if (status == -1)
			{
				perror("pclose");
				m_logger.LogInfo(LOG_NAME, "Unzipping 7z failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
				return false;
			} else {
				// printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}
		else
		{
			m_logger.LogInfo(LOG_NAME, "Found an existing 7Zip tool at " + std::string(m_7zToolPath));
		}

		return true;
	}

	bool Zip::Unzip7z(const fs::path& archiveFile, const fs::path& destinationDir, const std::string& password) const
	{
		m_logger.LogInfo(LOG_NAME, "Extracting " + std::string(archiveFile));
		if (!fs::exists(archiveFile))
		{
			return false;
		}

		// unzip
		FILE* fp;
		char strbuffer[512];
		snprintf(strbuffer, sizeof(strbuffer), "%s x -aoa -bsp2 -o%s -p%s %s 2>&1", m_7zToolPath.c_str(), destinationDir.c_str(), password.c_str(), archiveFile.c_str());
		const std::string dbgStr = strbuffer;
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			m_logger.LogError(LOG_NAME, "Unzipping archive failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			return false;
		}

		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) { }

		int status = pclose(fp);
		if (status == -1)
		{
			perror("pclose");
			m_logger.LogError(LOG_NAME, "Unzipping archive failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			return false;	// TODO: report this through a callback
		}
		else
		{
			// printf("7zz Command exited with status: %d\n", WEXITSTATUS(status));
			m_logger.LogInfo(LOG_NAME, "Extracting " + std::string(archiveFile) + " completed.");
		}

		return true;
	}
}
