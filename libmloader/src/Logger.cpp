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

#include "Logger.h"
#include <exception>
#include <ctime>

namespace mloader
{
	static std::string GetCurrentTime()
	{
		std::time_t now = std::time(nullptr);
		char buffer[255];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		return buffer;
	}

	Logger::Logger(const std::string& fileName)
	{
		m_logFile.open(fileName, std::ios::out | std::ios::app);
		if (!m_logFile.is_open())
		{
			throw std::runtime_error("Unable to open log file " + fileName);
		}
	}
	
	Logger::~Logger()
	{
		if (m_logFile.is_open())
		{
			m_logFile.close();
		}
	}

	void Logger::WriteToFile(const std::string& message)
	{
		std::lock_guard<std::mutex> lock(m_logMutex);
		m_logFile << message << std::endl;
	}

	void Logger::LogInfo(const std::string& system, const std::string& message)
	{
		const std::string msg = "[INFO] " + GetCurrentTime() + " [" + system + "]: " + message;
		WriteToFile(msg);
	}

	void Logger::LogWarning(const std::string& system, const std::string& message)
	{
		const std::string msg = "[WARNING] " + GetCurrentTime() + " [" + system + "]: " + message;
		WriteToFile(msg);
	}
	
	void Logger::LogError(const std::string& system, const std::string& message)
	{
		const std::string msg = "[ERROR] " + GetCurrentTime() + " [" + system + "]: " + message;
		WriteToFile(msg);
	}
}
