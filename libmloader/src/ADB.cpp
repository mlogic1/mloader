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

#include "ADB.h"
#include "Logger.h"
#include "Utility.h"
#include "curl_global.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <exception>
#include <iostream>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <vector>

namespace mloader
{
	ADB::ADB(const std::string& cacheDir, Logger& logger, std::function<void()> AdbDeviceListChangedCallback)
		:	m_cacheDir(cacheDir),
			m_logger(logger),
			m_adbDeviceListChangedCallback(AdbDeviceListChangedCallback)
	{
		CheckAndDownloadTool();
		ResetServer();
		StartBackgroundDeviceService();
	}

	ADB::~ADB()
	{
		m_endBackgroundService = true;
		KillServer();
	}

	std::vector<AdbDevice*> ADB::GetAdbDevices()
	{
		std::lock_guard<std::mutex> lock(m_devicesMutex);
		std::vector<AdbDevice*> devices;

		for (AdbDevice& device : m_devices)
		{
			devices.push_back(&device);
		}

		return devices;
	}

	void ADB::InstallFilesToDevice(const std::string& packageName, const std::vector<fs::path>& fileList, const AdbDevice& device) const
	{
		if (std::count_if(fileList.cbegin(), fileList.cend(), [](const fs::path& path)
		{
			return path.extension() == ".obb";
		}) > 0)
		{
			// If file list contains .obb files, clear and recreate obb directory
			const fs::path obbDir = fs::path("/sdcard/Android/obb/") / packageName;
			ExecShell(m_adbToolPath.c_str(), "-s", device.DeviceId, "shell", "rm", "-rf", obbDir);
			ExecShell(m_adbToolPath.c_str(), "-s", device.DeviceId, "shell", "mkdir", obbDir);	// mkdir is probably not needed but it doesn't hurt (adb push would create the directory regardless)
		}
		
		for (const fs::path& file : fileList)
		{
			const std::string extension = file.extension();
			const std::string fileName = file.filename();

			if (extension == ".apk")
			{
				InstallAPK(file, device.DeviceId);
			}
			else if (extension == ".obb")
			{
				InstallOBB(packageName, file, device.DeviceId);
			}
			else
			{
				std::string errMsg = "Unhandled file extension for file " + fileName + ". Installation will continue, but the application might not work";
				m_logger.LogError(LOG_NAME, errMsg);
			}
		}
	}

	std::vector<std::string> ADB::GetDeviceThirdPartyPackages(const AdbDevice& device) const
	{
		std::vector<std::string> packages;
		ExecShellWithCallback([&packages](const std::string& package)
		{
			std::string packageTrimmed = package.substr(sizeof("package:") - 1, package.length() - sizeof("package:"));
			if (packageTrimmed.back() == '\r')
			{
				packageTrimmed.pop_back();
			}
			packages.push_back(packageTrimmed);
		},
		m_adbToolPath, "-s", device.DeviceId, "shell", "pm", "list", "packages", "-3");
		return packages;
	}

	bool ADB::CheckAndDownloadTool()
	{
		const fs::path adbToolDir = m_cacheDir / "platform-tools/";
		m_adbToolPath = adbToolDir/ "adb";
	#ifdef _WIN32
		// const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-windows.zip"};
		// no support for windows yet
	#elif __APPLE__
		// x86 Intel (no support for apple arm yet.)
		const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-darwin.zip"};
		const fs::path adbToolPathZip = m_cacheDir / "platform-tools-latest-darwin.zip";
		
	#else
		const std::string httpFile{"https://dl.google.com/android/repository/platform-tools-latest-linux.zip"};
		const fs::path adbToolPathZip = m_cacheDir / "platform-tools-latest-linux.zip";
	#endif

		if (!fs::exists(m_adbToolPath))
		{
			if (!fs::exists(adbToolPathZip))
			{
				m_logger.LogInfo(LOG_NAME, "Downloading ADB");
				if (!CurlDownloadFile(httpFile, adbToolPathZip))
				{
					throw std::runtime_error("Unable to download ADB");
				}
			}

			// extract
			FILE* fp;
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), "unzip %s -d %s", adbToolPathZip.c_str(), m_cacheDir.c_str());
			const std::string dbgStr = buffer;
			fp = popen(buffer, "r");
			if (fp == NULL)
			{
				m_logger.LogError(LOG_NAME, "Unzipping ADB failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
				perror("popen");
				return false;
			}
			
			char path[1035];
			// Read the output a line at a time - output it.
			while (fgets(path, sizeof(path), fp) != NULL) {
				// printf("%s", path);
			}

			int status = pclose(fp);
			if (status == -1)
			{
				m_logger.LogError(LOG_NAME, "Unzipping ADB failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
				perror("pclose");
				return false;
			} 
			else
			{
				// printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}
		else
		{
			m_logger.LogInfo(LOG_NAME, "Found an existing ADB tool at " + std::string(m_adbToolPath));
		}

		return true;
	}

	void ADB::StartServer()
	{
		while(m_backgroundServiceBusy) { } // wait for the background service to finish (just in case)
		
		m_logger.LogInfo(LOG_NAME, "Starting ADB Server");
		
		FILE* fp;
		char strbuffer[512];

		snprintf(strbuffer, sizeof(strbuffer), "%s start-server", m_adbToolPath.c_str());
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			m_logger.LogError(LOG_NAME, "Starting ADB Server failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			perror("popen");
		}
		
		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			m_logger.LogInfo(LOG_NAME, path);
		}

		int status = pclose(fp);
		if (status != EXIT_SUCCESS)
		{
			m_logger.LogError(LOG_NAME, "Starting ADB Server failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			perror("pclose");
		}
		else
		{
			m_logger.LogInfo(LOG_NAME, "ADB Server started");
		}
	}
	
	void ADB::ResetServer()
	{
		KillServer();
		StartServer();
	}
	
	void ADB::KillServer()
	{
		while(m_backgroundServiceBusy) { } // wait for the background service to finish
		
		m_logger.LogInfo(LOG_NAME, "Stopping ADB Server");
		
		FILE* fp;
		char strbuffer[512];

		snprintf(strbuffer, sizeof(strbuffer), "%s kill-server", m_adbToolPath.c_str());
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			m_logger.LogError(LOG_NAME, "Stopping ADB Server failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			perror("popen");
			// return false;
		}
		
		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			m_logger.LogInfo(LOG_NAME, path);
		}

		int status = pclose(fp);
		if (status == -1)
		{
			m_logger.LogError(LOG_NAME, "Stopping ADB Server failed. Error no: " + std::to_string(errno) + ". " + strerror(errno));
			perror("pclose");
		}
		else
		{
			m_logger.LogInfo(LOG_NAME, "Stopped ADB Server");
		}
	}

	static const std::string GetDeviceModel(const std::string& adbExecutablePath, const std::string& deviceId)
	{
		constexpr const char* shellCommandFormat = "%s -s %s shell getprop ro.product.model";
		char strBuffer[255];
		snprintf(strBuffer, sizeof(strBuffer), shellCommandFormat, adbExecutablePath.c_str(), deviceId.c_str());
		
		FILE* fp = popen(strBuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
		}
		
		char modelBuffer[255];
		fgets(modelBuffer, sizeof(modelBuffer), fp);
		
		int status = pclose(fp);
		if (status == -1) {
			perror("pclose");
		}
		
		std::string result(modelBuffer);
		result.erase(std::remove_if(result.begin(), result.end(), [](char c) { return c == '\n' || c == '\r'; }), result.end());
		return result;
	}

	void ADB::StartBackgroundDeviceService()
	{
		std::thread([&]() {
			while(!m_endBackgroundService)
			{
				std::this_thread::sleep_for(std::chrono::seconds(2));
				m_backgroundServiceBusy = true;
				FILE* fp;
				char strbuffer[512];

				snprintf(strbuffer, sizeof(strbuffer), "%s devices", m_adbToolPath.c_str());
				fp = popen(strbuffer, "r");
				if (fp == NULL)
				{
					perror("popen");
					// return false;
				}
				
				// m_logger.LogInfo(LOG_NAME, Refreshing device list);
				
				char path[1035];
				std::vector<std::string> adbDevices;	// list from shell
				// Read the output
				while (fgets(path, sizeof(path), fp) != NULL) {
					adbDevices.push_back(path);
				}

				int status = pclose(fp);
				if (status != EXIT_SUCCESS) {
					m_logger.LogError(LOG_NAME, "ADB Exited with status " + std::to_string(status) + ". Error number " + std::to_string(errno) + ". " + strerror(errno));
					perror("pclose");
				}
				else
				{
					// printf("Command exited with status: %d\n", WEXITSTATUS(status));
				}

				if (adbDevices.empty())
				{
					continue;
				}

				adbDevices.erase(adbDevices.begin());	// remove header line
				
				adbDevices.erase(std::find_if(adbDevices.begin(), adbDevices.end(), [](const std::string& deviceLine)
				{
					return deviceLine.empty() || deviceLine == "\n";
				}));
				
				decltype(m_devices) deviceList;
				
				for (std::string adbDeviceLine : adbDevices)
				{
					adbDeviceLine.erase(std::remove(adbDeviceLine.begin(), adbDeviceLine.end(), '\n'), adbDeviceLine.cend());
					std::istringstream iss(adbDeviceLine);
					std::string deviceId, deviceStatus, deviceModel;	// Note: model can only be obtained for authorized devices
					AdbDeviceStatus eDeviceStatus = AdbDeviceStatus::Unknown;
					std::getline(iss, deviceId, '\t');
					std::getline(iss, deviceStatus, '\t');
					
					// possible device statuses:
					// device - connected and authorized
					// unauthorized - requires authorization
					// offline - device was connected but went offline
					// no permissions - no debuggging permissions
					
					if (deviceStatus == "device")
					{
						deviceModel = GetDeviceModel(m_adbToolPath, deviceId);
					}

					// "no permissions" status might contain trailing text, trim it
					if (deviceStatus.starts_with("no permissions"))
					{
						deviceStatus = "no permissions";
					}
					
					static const std::unordered_map<std::string, AdbDeviceStatus> DEVICE_STATUS_MAP =
					{
						{ "device", AdbDeviceStatus::OK 					},
						{ "unauthorized", AdbDeviceStatus::UnAuthorized		},
						{ "offline", AdbDeviceStatus::Offline 				},
						{ "no permissions", AdbDeviceStatus::NoPermissions	}
					};
					
					try
					{
						eDeviceStatus = DEVICE_STATUS_MAP.at(deviceStatus);
					}
					catch (std::out_of_range& exception)
					{
						m_logger.LogError(LOG_NAME, "Unknown device status reported on device list: " + deviceStatus + ". " + std::string(exception.what()));
					}
					
					deviceList.push_back({
						 .DeviceId = strdup(deviceId.c_str()),
						 .Model = strdup(deviceModel.c_str()),
						 .DeviceStatus = eDeviceStatus
					});
				}
				
				if (deviceList != m_devices)
				{
					m_logger.LogInfo(LOG_NAME, "Device change detected");
					{
						std::lock_guard<std::mutex> lock(m_devicesMutex);
						for(int i = 0; i < m_devices.size(); ++i){ DestroyAdbDevice(m_devices[i]); }
						m_devices.clear();
						m_devices = std::move(deviceList);
					}
					if (m_adbDeviceListChangedCallback)
					{
						m_adbDeviceListChangedCallback();
					}
				}
				
				m_backgroundServiceBusy = false;
			}
		}).detach();
	}

	bool ADB::InstallAPK(const fs::path& file, const char* serial) const
	{
		m_logger.LogInfo(LOG_NAME, "Installing APK " + file.string() + " to device " + serial);
		std::string escapedAPK = "\"" + file.string() + "\"";
		return ExecShell(m_adbToolPath.c_str(), "-s", serial, "install", "-r", escapedAPK.c_str()) == 0;
	}

	bool ADB::InstallOBB(const std::string& packageName, const fs::path& file, const char* serial) const
	{
		m_logger.LogInfo(LOG_NAME, "Installing OBB " + file.string() + " to device " + serial);
		fs::path targetLocation = fs::path("/sdcard/Android/obb/") / packageName / file.filename();
		std::string escapedOBB = "\"" + file.string() + "\"";
		return ExecShell(m_adbToolPath.c_str(), "-s", serial, "push", escapedOBB.c_str(), targetLocation) == 0;
	}
}
