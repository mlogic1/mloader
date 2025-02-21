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

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace mloader
{
	class Logger
	{
		public:
			Logger(const std::string& fileName);
			~Logger();
		
			void LogInfo(const std::string& system, const std::string& message);
			void LogWarning(const std::string& system, const std::string& message);
			void LogError(const std::string& system, const std::string& message);
		
		private:
			void WriteToFile(const std::string& message);
		private:
			std::ofstream m_logFile;
			std::mutex m_logMutex;
	};
}

#endif // LOGGER_H
