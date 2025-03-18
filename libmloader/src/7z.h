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

#ifndef SEVENZ_H
#define SEVENZ_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace mloader
{
	class Logger;
	class Zip
	{
		public:
			Zip(const std::string& cacheDir, Logger& logger);
			~Zip();

			bool Unzip7z(const fs::path& archiveFile, const fs::path& destinationDir, const std::string& password = "") const;

		private:
			void CheckAndDownloadTool();

		private:
			fs::path m_cacheDir;
			fs::path m_7zToolPath;	// points to 7z executable

			Logger& m_logger;
			static constexpr const char* LOG_NAME = "Zip";
	};
}

#endif // SEVENZ_H
