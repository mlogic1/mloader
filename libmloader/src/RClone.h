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

#ifndef RCLONE_H
#define RCLONE_H

#include <string>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace mloader
{
	class Logger;
	class RClone
	{
		public:
			RClone(const std::string& cacheDir, Logger& logger);
			~RClone();

			bool SyncFile(const std::string& baseUrl, const std::string& fileName, const fs::path& directory) const;
			bool CopyFile(const std::string& baseUrl, const std::string& fileId, const fs::path& directory, std::function<void(uint8_t)> progressCallback = nullptr) const;
		
		private:
			void CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
			fs::path m_rcloneToolPath;	// points to rclone executable

			Logger& m_logger;
			static constexpr const char* LOG_NAME = "RClone";
	};
}

#endif // RCLONE_H
