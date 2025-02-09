#include "ADB.h"
#include "curl_global.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <exception>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <vector>

namespace mloader
{
	ADB::ADB(const std::string& cacheDir, std::function<void()> AdbDeviceListChangedCallback)
		:	m_cacheDir(cacheDir),
			m_adbDeviceListChangedCallback(AdbDeviceListChangedCallback)
	{
		CheckAndDownloadTool();
		ResetServer();
		StartBackgroundDeviceService();
	}

	ADB::~ADB()
	{
		KillServer();
		m_endBackgroundService = true;
	}

	std::vector<AdbDevice> ADB::GetAdbDevices()
	{
		std::lock_guard<std::mutex> lock(m_devicesMutex);
		return m_devices;
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
				// log downloading adb
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
				perror("popen");
				return false;
			}
			
			char path[1035];
			// Read the output a line at a time - output it.
			while (fgets(path, sizeof(path), fp) != NULL) {
				// printf("%s", path);
			}

			int status = pclose(fp);
			if (status == -1) {
				perror("pclose");
				return false;
			} 
			else
			{
				// printf("Command exited with status: %d\n", WEXITSTATUS(status));
			}
		}

		return true;
	}

	void ADB::StartServer()
	{
		while(m_backgroundServiceBusy)
		{
			// wait for the background service to finish (just in case)
		}
		
		FILE* fp;
		char strbuffer[512];

		snprintf(strbuffer, sizeof(strbuffer), "%s start-server", m_adbToolPath.c_str());
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			// return false;
		}
		
		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			// printf("%s", path);
		}

		int status = pclose(fp);
		if (status == -1) {
			perror("pclose");
			// return false;
		}
		else
		{
			// printf("Command exited with status: %d\n", WEXITSTATUS(status));
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
		
		FILE* fp;
		char strbuffer[512];

		snprintf(strbuffer, sizeof(strbuffer), "%s kill-server", m_adbToolPath.c_str());
		fp = popen(strbuffer, "r");
		if (fp == NULL)
		{
			perror("popen");
			// return false;
		}
		
		char path[1035];
		// Read the output a line at a time - output it.
		while (fgets(path, sizeof(path), fp) != NULL) {
			// printf("%s", path);
		}

		int status = pclose(fp);
		if (status == -1) {
			perror("pclose");
			// return false;
		}
		else
		{
			// printf("Command exited with status: %d\n", WEXITSTATUS(status));
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
				constexpr const char* searchHeader = "List of devices attached";

				snprintf(strbuffer, sizeof(strbuffer), "%s devices", m_adbToolPath.c_str());
				fp = popen(strbuffer, "r");
				if (fp == NULL)
				{
					perror("popen");
					// return false;
				}
				
				printf("Refreshing device list\n");
				
				char path[1035];
				std::vector<std::string> adbDevices;	// list from shell
				// Read the output
				while (fgets(path, sizeof(path), fp) != NULL) {
					adbDevices.push_back(path);
				}
				
				adbDevices.erase(adbDevices.begin());	// remove header line
				
				adbDevices.erase(std::find_if(adbDevices.begin(), adbDevices.end(), [](const std::string& deviceLine)
				{
					return deviceLine.empty() || deviceLine == "\n";
				}));

				int status = pclose(fp);
				if (status == -1) {
					perror("pclose");
					// return false;
				}
				else
				{
					// printf("Command exited with status: %d\n", WEXITSTATUS(status));
				}
				
				decltype(m_devices) deviceList;
				
				for (std::string adbDeviceLine : adbDevices)
				{
					adbDeviceLine.erase(std::remove(adbDeviceLine.begin(), adbDeviceLine.end(), '\n'), adbDeviceLine.cend());
					// adbDeviceLine.
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
					
					static const std::unordered_map<std::string, AdbDeviceStatus> DEVICE_STATUS_MAP =
					{
						{ "device", AdbDeviceStatus::OK },
						{ "unauthorized", AdbDeviceStatus::UnAuthorized },
						{ "offline", AdbDeviceStatus::Offline },
						{ "no permissions", AdbDeviceStatus::NoPermissions }
					};
					
					try
					{
						eDeviceStatus = DEVICE_STATUS_MAP.at(deviceStatus);
					}
					catch (std::out_of_range&)
					{
						// TODO: log this
					}
					
					deviceList.push_back({
						 .DeviceId = strdup(deviceId.c_str()),
						 .Model = strdup(deviceModel.c_str()),
						 .DeviceStatus = eDeviceStatus
					});
				}
				
				if (deviceList != m_devices)
				{
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
}
