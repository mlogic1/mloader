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

#include <mloader/AppContext.h>
#include "Logger.h"
#include "VRPManager.h"
#include "RClone.h"
#include "ADB.h"
#include "7z.h"
#include "curl_global.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include <thread>
#include <future>
#include <unordered_map>

namespace fs = std::filesystem;

struct AppContext
{
	mloader::VRPManager*			VrpManager;
	mloader::RClone*				Rclone;
	mloader::Zip*					Zip7;
	mloader::ADB*					Adb;
	mloader::Logger*				Logger;

	// App list
	App** 							AppList = nullptr;
	AdbDevice**						AdbDeviceList = nullptr;
	
	// callbacks
	ADBDeviceListChangedCallback	AdbDeviceListChangedCallback			= nullptr;
	void*							AdbDeviceListChangedCallbackUserData	= nullptr;
	AppStatusChangedCallback		AppsStatusChangedCallback				= nullptr;
	void*							AppsStatusChangedCallbackUserData		= nullptr;
};

static std::string DetermineCacheDir()
{
	#ifdef _WIN32
		// TODO: implement windows
		m_cacheDir = something
		m_downloadDir = something
	#else
		return "/tmp/mloader/cache";
	#endif
}

static std::string DetermineDownloadDir()
{
	#ifdef _WIN32
		// TODO: implement windows
		m_cacheDir = something
		m_downloadDir = something
	#else
		const std::string homedir = std::getenv("HOME");
		return homedir + std::string("/MLoaderDownloads");
	#endif
}

static const char* err_msg;

static const char* GetErrorMessage()
{
	return err_msg;
}

static inline void GenericCallback(CreateLoaderContextStatusCallback callback, const char* message)
{
	if (callback != NULL)
	{
		callback(message);
	}
}

static int RefreshAdbDeviceList(AppContext* context)
{
	if (context->AdbDeviceList)
	{
		delete[] context->AdbDeviceList;
		context->AdbDeviceList = nullptr;
	}

	std::vector<AdbDevice*> adbDevices = context->Adb->GetAdbDevices();
	int numDevices = static_cast<int>(adbDevices.size());
	
	context->AdbDeviceList = new AdbDevice*[numDevices];
	
	for(int i = 0; i < numDevices; ++i)
	{
		context->AdbDeviceList[i] = adbDevices[i];
	}

	return numDevices;
}

static char* GetUpdateStatusString(AppStatus appStatus, int statusParam = -1)
{
	static std::unordered_map<AppStatus, const char*> APP_STATUS_STR_MAP =
	{
		{ AppStatus::NoInfo, 			"" 					},
		{ AppStatus::Downloading, 		"Downloading" 		},
		{ AppStatus::DownloadError, 	"Download Error"	},
		{ AppStatus::Extracting, 		"Extracting"		},
		{ AppStatus::ExtractingError,	"Extracting Error"	},
		{ AppStatus::Downloaded,		"Downloaded"		},
		{ AppStatus::Installing,		"Installing"		},
		{ AppStatus::InstallingError,	"Installing Error"	},
		{ AppStatus::Installed,			"Installed"			}
	};

	char* result = nullptr;
	if (appStatus == AppStatus::Downloading)
	{
		asprintf(&result, "%s (%d%%)", APP_STATUS_STR_MAP.at(appStatus), statusParam);
	}
	else
	{
		result = strdup(APP_STATUS_STR_MAP.at(appStatus));
	}
	
	return result;
}

void OnAdbDeviceListChangedEvent(AppContext* context)
{
	RefreshAdbDeviceList(context);

	if (context->AdbDeviceListChangedCallback)
	{
		context->AdbDeviceListChangedCallback(context, context->AdbDeviceListChangedCallbackUserData);
	}
}

void OnGameInfoStatusChanged(AppContext* context, const mloader::GameInfo& gameInfo, const AppStatus appStatus, const int statusParam)
{
	if (context->AppsStatusChangedCallback && context->AppList != nullptr)
	{
		App* updatedApp = nullptr;
		for (int i = 0; i < context->VrpManager->GetGameList().size(); ++i)
		{
			if (context->AppList[i]->ReleaseName == gameInfo.ReleaseName)
			{
				updatedApp = context->AppList[i];
				updatedApp->Status = appStatus;
				free((char*)context->AppList[i]->StatusCStr);
				context->AppList[i]->StatusCStr = GetUpdateStatusString(appStatus, statusParam);
				context->AppsStatusChangedCallback(context, updatedApp, context->AppsStatusChangedCallbackUserData);
				break;
			}
		}
	}
}

AppContext* CreateLoaderContext(CreateLoaderContextStatusCallback callback, const char* customCacheDir, const char* customDownloadDir)
{
	GenericCallback(callback, "Initializing");
	
	mloader::InitGlobalCurl();
	fs::path cacheDir = DetermineCacheDir();
	fs::path downloadDir = DetermineDownloadDir();

	if (customCacheDir[0] != '\0')
	{
		cacheDir = customCacheDir;
	}

	if (customDownloadDir[0] != '\0')
	{
		downloadDir = customDownloadDir;
	}

	if (!fs::exists(cacheDir))
	{
		if (!fs::create_directories(cacheDir))
		{
			err_msg = "Unable to create cache directory.";
			return NULL;
		}
	}

	if (!fs::exists(downloadDir))
	{
		if (!fs::create_directories(downloadDir))
		{
			err_msg = "Unable to create download directory.";
			return NULL;
		}
	}

	AppContext* appContext = new AppContext();

	try
	{
		appContext->Logger = new mloader::Logger(cacheDir / "mloader.log");
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext;
		return nullptr;
	}
	
	appContext->Logger->LogInfo("Initialization", "Starting up");
	appContext->Logger->LogInfo("Initialization", "Cache directory: " + std::string(cacheDir));
	appContext->Logger->LogInfo("Initialization", "Download directory: " + std::string(downloadDir));
	
	try
	{
		GenericCallback(callback, "Initializing RClone");
		appContext->Rclone = new mloader::RClone(cacheDir, *appContext->Logger);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Logger;
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Initializing 7-Zip");
		appContext->Zip7 = new mloader::Zip(cacheDir, *appContext->Logger);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->Logger;
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Initializing ADB");
		appContext->Adb = new mloader::ADB(cacheDir, *appContext->Logger, std::bind(OnAdbDeviceListChangedEvent, appContext));
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->Zip7;
		delete appContext->Logger;
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Initializing VRP");

		auto onAppStatusChanged = [appContext](const mloader::GameInfo& gameInfo, const AppStatus appStatus, const int statusParam)
		{
			OnGameInfoStatusChanged(appContext, gameInfo, appStatus, statusParam);
		};
		
		appContext->VrpManager = new mloader::VRPManager(*appContext->Rclone, *appContext->Zip7, cacheDir, downloadDir, *appContext->Logger, onAppStatusChanged);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->Zip7;
		delete appContext->Adb;
		delete appContext->Logger;
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Loading metadata");
		RefreshMetadata(appContext);
	}catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->Zip7;
		delete appContext->Adb;
		delete appContext->VrpManager;
		delete appContext->Logger;
		delete appContext;
		return nullptr;
	}

	return appContext;
}

void CreateLoaderContextAsync(CreateLoaderContextAsyncCompletedCallback completedCallback, CreateLoaderContextStatusCallback callback, const char* customCacheDir, const char* customDownloadDir)
{
	std::thread([=]() {
		std::future<AppContext*> ret = std::async(std::launch::async, CreateLoaderContext, callback, customCacheDir, customDownloadDir);
		AppContext* result = ret.get();

		completedCallback(result);
	}).detach();
}

void DestroyLoaderContext(AppContext* context)
{
	if (context->AppList)
	{
		const size_t numApps = context->VrpManager->GetGameList().size();
		for (int i = 0; i < numApps; ++i)
		{
			// cleanup individual strings
			free((char*)context->AppList[i]->GameName);
			free((char*)context->AppList[i]->ReleaseName);
			free((char*)context->AppList[i]->PackageName);
			free((char*)context->AppList[i]->LastUpdated);
			free((char*)context->AppList[i]->StatusCStr);

			context->AppList[i]->GameName		= NULL;
			context->AppList[i]->ReleaseName	= NULL;
			context->AppList[i]->PackageName	= NULL;
			context->AppList[i]->LastUpdated	= NULL;
			context->AppList[i]->StatusCStr		= NULL;

			delete context->AppList[i];
		}
		delete[] context->AppList;
	}

	if (context->AdbDeviceList)
	{
		delete[] context->AdbDeviceList;
		context->AdbDeviceList = nullptr;
	}

	context->AdbDeviceListChangedCallback		= nullptr;
	context->AppsStatusChangedCallback			= nullptr;
	context->AppsStatusChangedCallbackUserData	= nullptr;

	if (context != nullptr)
	{
		delete context->VrpManager;
		delete context->Adb;
		delete context->Zip7;
		delete context->Rclone;
		delete context->Logger;
		delete context;
		context = nullptr;
	}
	mloader::CleanupGlobalCurl();
}

void RefreshMetadata(AppContext* context)
{
	if (!context->VrpManager->RefreshMetadata())
	{
		err_msg = "Unable to download or load metadata";
	}
}

void RefreshMetadataAsync(RefreshMetadataAsyncCompletedCallback completedCallback, AppContext* context)
{
	std::thread([=]() {
		std::future<void> ret = std::async(std::launch::async, RefreshMetadata, context);

		completedCallback(context);
	}).detach();
}

App** GetAppList(AppContext* context, int* num)
{
	if (context->AppList == nullptr)	// lazy load
	{
		const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();
		*num = gameInfo.size();
		context->AppList = new App*[*num];
		int i = 0;
		for (const auto& pair : gameInfo)
		{
			context->AppList[i] = new App();
			context->AppList[i]->GameName 		= strdup(pair.first.GameName.c_str());
			context->AppList[i]->ReleaseName 	= strdup(pair.first.ReleaseName.c_str());
			context->AppList[i]->PackageName 	= strdup(pair.first.PackageName.c_str());
			context->AppList[i]->VersionCode 	= pair.first.VersionCode;
			context->AppList[i]->LastUpdated 	= strdup(pair.first.LastUpdated.c_str());
			context->AppList[i]->SizeMB 		= pair.first.SizeMB;
			context->AppList[i]->Downloads 		= pair.first.Downloads;
			context->AppList[i]->Rating 		= pair.first.Rating;
			context->AppList[i]->RatingCount 	= pair.first.RatingCount;
			context->AppList[i]->Status 		= pair.second;
			context->AppList[i]->AppStatusParam = -1;									// When downloading or extracting, progress is reported with this param, otherwise it defaults to -1
			context->AppList[i]->StatusCStr = GetUpdateStatusString(context->AppList[i]->Status);

			++i;
		}
	}

	return context->AppList;
}

bool DownloadApp(AppContext* context, App* app)
{
	const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();

	auto it = std::find_if(gameInfo.cbegin(), gameInfo.cend(), [app](const auto& pair)
	{
		return pair.first.ReleaseName == app->ReleaseName;
	});

	if (it == gameInfo.end())
	{
		return false;
	}

	context->VrpManager->DownloadGame(it->first);
	return true;
}

void DownloadAppAsync(AppContext* context, App* app)
{
	std::thread([=]() {
		DownloadApp(context, app);
	}).detach();
}

bool MLoaderInstallApp(AppContext* context, App* app, AdbDevice* device)
{
	const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();

	auto it = std::find_if(gameInfo.cbegin(), gameInfo.cend(), [app](const auto& pair)
	{
		return pair.first.ReleaseName == app->ReleaseName;
	});

	if (it == gameInfo.end())
	{
		return false;
	}

	if (device->DeviceStatus != AdbDeviceStatus::OK)
	{
		err_msg = "Device not authorized";
		return false;
	}

	try
	{
		context->VrpManager->UpdateGameStatus(it->first, AppStatus::Installing);
		std::vector<fs::path> fileList = context->VrpManager->GetGameFileList(it->first);
		context->Adb->InstallFilesToDevice(it->first.PackageName, fileList, *device);
		context->VrpManager->UpdateGameStatus(it->first, AppStatus::Installed);
	}
	catch(std::runtime_error& err)
	{
		context->VrpManager->UpdateGameStatus(it->first, AppStatus::InstallingError);
		err_msg = err.what();
		return false;
	}

	return true;
}

void MLoaderInstallAppAsync(AppContext* context, App* app, AdbDevice* device)
{
	std::thread([=]() {
		MLoaderInstallApp(context, app, device);
	}).detach();
}

AdbDevice** GetDeviceList(AppContext* context, int* num)
{
	if (context->AdbDeviceList)
	{
		*num = context->Adb->GetAdbDevices().size();
		return context->AdbDeviceList;
	}
	
	*num = RefreshAdbDeviceList(context);

	return context->AdbDeviceList;
}

void SetADBDeviceListChangedCallback(AppContext* context, ADBDeviceListChangedCallback callback, void* userData)
{
	context->AdbDeviceListChangedCallback = callback;
	context->AdbDeviceListChangedCallbackUserData = userData;
}

void ClearADBDeviceListChangedCallback(AppContext* context)
{
	context->AdbDeviceListChangedCallback = nullptr;
	context->AdbDeviceListChangedCallbackUserData = nullptr;
}

void SetAppStatusChangedCallback(AppContext* context, AppStatusChangedCallback callback, void* userData)
{
	context->AppsStatusChangedCallback = callback;
	context->AppsStatusChangedCallbackUserData = userData;
}

void ClearAppStatusChangedCallback(AppContext* context)
{
	context->AppsStatusChangedCallback = nullptr;
	context->AppsStatusChangedCallbackUserData = nullptr;
}

char* GetAppThumbImage(AppContext* context, App* app)
{
	const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();

	auto it = std::find_if(gameInfo.cbegin(), gameInfo.cend(), [app](const auto& pair)
	{
		return pair.first.ReleaseName == app->ReleaseName;
	});

	if (it == gameInfo.end())
	{
		return NULL;
	}
	
	const std::string path = context->VrpManager->GetAppThumbImage(it->first);
	if (path.empty())
	{
		return NULL;
	}
	char* cpath = strdup(path.c_str());
	return cpath;
}
