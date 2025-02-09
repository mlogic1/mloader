#include "RClone.h"
#include "curl_global.h"
#include <filesystem>
#include <exception>
#include <iostream>
#include <regex>
#include <fstream>

namespace mloader
{
	RClone::RClone(const std::string& cacheDir)
		: m_cacheDir(cacheDir)
	{
		CheckAndDownloadTool();
	}

	RClone::~RClone()
	{

	}

	bool RClone::CheckAndDownloadTool()
	{
	#ifdef _WIN32
		// const std::string httpFile{"https://downloads.rclone.org/v1.69.0/rclone-v1.69.0-windows-amd64.zip"};
		// no support for windows yet
	#elif __APPLE__
		// x86 Intel (no support for apple arm yet.)
		const std::string httpFile{"https://downloads.rclone.org/v1.69.0/rclone-v1.69.0-osx-amd64.zip"};

		const fs::path rcloneToolDir = m_cacheDir / "rclone-v1.69.0-osx-amd64/";
		const fs::path rcloneToolPathZip = m_cacheDir / "rclone-v1.69.0-osx-amd64.zip";
	#else
		const std::string httpFile{"https://downloads.rclone.org/v1.69.0/rclone-v1.69.0-linux-amd64.zip"};

		const fs::path rcloneToolDir = m_cacheDir / "rclone-v1.69.0-linux-amd64/";
		const fs::path rcloneToolPathZip = m_cacheDir / "rclone-v1.69.0-linux-amd64.zip";
	#endif
		
		m_rcloneToolPath = rcloneToolDir / "rclone";

		if (!fs::exists(m_rcloneToolPath))
		{
			if (!fs::exists(rcloneToolPathZip))
			{
				// log downloading rclone
				if (!CurlDownloadFile(httpFile, rcloneToolPathZip))
				{
					throw std::runtime_error("Unable to download RClone");
				}
			}

			// extract
			FILE* fp;
			char strbuffer[1024];
			snprintf(strbuffer, sizeof(strbuffer), "unzip %s -d %s", rcloneToolPathZip.c_str(), m_cacheDir.c_str());
			const std::string dbgStr = strbuffer;
			fp = popen(strbuffer, "r");
			if (fp == NULL)
			{
				perror("popen");
				return false;
			}
			
			char path[1035];
			// Read the output a line at a time - although don't output it.
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
				printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}

		return true;
	}

	bool RClone::SyncFile(const std::string& baseUrl, const std::string& fileName, const fs::path& directory) const
	{
			FILE* fp;
			char strbuffer[512];

			snprintf(strbuffer, sizeof(strbuffer), "%s --http-url %s --tpslimit 1.0 --tpslimit-burst 3 sync \":http:/%s\" \"%s\"", m_rcloneToolPath.c_str(), baseUrl.c_str(), fileName.c_str(), directory.c_str());
			const std::string dbgStr{strbuffer};
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
			} 
			else
			{
				printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}

		return true;
	}
	
	bool RClone::CopyFile(const std::string& baseUrl, const std::string& fileId, const fs::path& directory) const
	{
		FILE* fp;
		char strbuffer[1024];

		const fs::path directoryWithSubdir = m_cacheDir / fileId;

		// prompt override here?

		snprintf(strbuffer, sizeof(strbuffer), "%s --http-url %s --tpslimit 1.0 --tpslimit-burst 3 copy \":http:/%s\" \"%s\" --transfers 1 --multi-thread-streams 0 --progress", m_rcloneToolPath.c_str(), baseUrl.c_str(), fileId.c_str(), directoryWithSubdir.c_str());
		const std::string dbgStr{strbuffer};
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			return false;
		}
		
		char path[2048];
		system("clear");

		std::regex percentage_regex(R"((\d+)%(?=, ))"); // detect progress
		std::smatch match;

		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			std::string line(path);

			if (std::regex_search(line, match, percentage_regex))
			{
				std::cout << match[1] << "%" << std::endl;
				// TODO: report this through a callback
			}
		}

		int status = pclose(fp);
		if (status == -1) {
			perror("pclose");
			return false;
		} 
		else
		{
			printf("Command exited with status: %d\n", WEXITSTATUS(status));
			// TODO: report this through a callback
		}
		return true;
	}
}
