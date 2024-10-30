#ifndef CURL_GLOBAL_H
#define CURL_GLOBAL_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	void InitGlobalCurl();
	void CleanupGlobalCurl();

	std::string CurlGetRequest();
	bool CurlDownloadFile(const std::string& httpFile, const fs::path& destinationFile);
	
}

#endif	// CURL_GLOBAL_H