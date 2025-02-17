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
