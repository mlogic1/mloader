#ifndef ADB_H
#define ADB_H

#include "AdbDeviceImpl.h"
#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace mloader
{
	class Logger;
	class ADB
	{
		public:
			ADB(const std::string& cacheDir, Logger& logger, std::function<void()> AdbDeviceListChangedCallback = nullptr);
			~ADB();
		
			std::vector<AdbDevice> GetAdbDevices();
		
		private:
			bool CheckAndDownloadTool();
			void StartServer();
			void ResetServer();
			void KillServer();
			void StartBackgroundDeviceService();

		private:
			fs::path m_cacheDir;
			fs::path m_adbToolPath;	// points to adb executable
		
			std::vector<AdbDevice> m_devices;
			std::mutex m_devicesMutex;
		
			std::function<void()> m_adbDeviceListChangedCallback;
		
			std::atomic_bool m_endBackgroundService{false};
			std::atomic_bool m_backgroundServiceBusy{false};

			Logger& m_logger;
			static constexpr const char* LOG_NAME = "ADB";
	};
}

#endif // ADB_H
