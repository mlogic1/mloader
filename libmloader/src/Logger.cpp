#include "Logger.h"
#include <exception>

namespace mloader
{
	Logger::Logger(const std::string& fileName)
	{
		m_logFile.open(fileName, std::ios::out | std::ios::app);
		if (!m_logFile.is_open())
		{
			throw std::runtime_error("Unable to open log file");
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
		const std::string msg = "[INFO] " + system + ": " + message;
		WriteToFile(msg);
	}

	void Logger::LogWarning(const std::string& system, const std::string& message)
	{
		const std::string msg = "[WARNING] " + system + ": " + message;
		WriteToFile(msg);
	}
	
	void Logger::LogError(const std::string& system, const std::string& message)
	{
		const std::string msg = "[ERROR] " + system + ": " + message;
		WriteToFile(msg);
	}
}
