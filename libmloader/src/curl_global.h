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