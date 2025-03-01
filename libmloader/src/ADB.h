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

			std::vector<AdbDevice*> GetAdbDevices();
			void InstallFilesToDevice(const std::string& packageName, const std::vector<fs::path>& fileList, const AdbDevice& device) const;
			std::vector<std::string> GetDeviceThirdPartyPackages(const AdbDevice& device) const;
			std::string GetDeviceProperty(const AdbDevice& device, const std::string propName) const;

		private:
			bool CheckAndDownloadTool();
			void StartServer();
			void ResetServer();
			void KillServer();
			void StartBackgroundDeviceService();

			bool InstallAPK(const fs::path& file, const char* serial) const;
			bool InstallOBB(const std::string& packageName, const fs::path& file, const char* serial) const;

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
