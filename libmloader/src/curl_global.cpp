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

#include "curl_global.h"
#include <curl/curl.h>
#include <fstream>

namespace mloader
{
	static bool curl_initialized = false;

	static size_t CurlWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream)
	{
		std::ofstream* out = static_cast<std::ofstream*>(stream);
		out->write(static_cast<char*>(ptr), size * nmemb);
		return size * nmemb;
	}

	std::string CurlGetRequest()
	{
		return std::string{"Not implemented"};
	}

	bool CurlDownloadFile(const std::string& httpFile, const fs::path& destinationFile)
	{
		CURL* curl = curl_easy_init();

		std::ofstream destFile(destinationFile);

		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, httpFile.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &destFile);
			CURLcode res = curl_easy_perform(curl);
			if (res != CURLE_OK)
			{
				// LOG error (maybe even throw an exception)
				destFile.close();
				return false;
			}
		}
		else
		{
			destFile.close();
			return false;
		}

		destFile.close();
		return true;
	}

	void InitGlobalCurl()
	{
		if (!curl_initialized)
		{
			curl_global_init(CURL_GLOBAL_DEFAULT);
			curl_initialized = true;
		}
	}

	void CleanupGlobalCurl()
	{
		if (curl_initialized)
		{
			curl_global_cleanup();
			curl_initialized = false;
		}
	}
}