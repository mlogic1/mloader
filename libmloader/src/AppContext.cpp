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
#include "QueueManager.h"
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
	mloader::QueueManager*			QueueManager;

	// App list
	VrpApp** 						AppList 								= nullptr;
	AdbDevice**						AdbDeviceList 							= nullptr;

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
		return "/tmp/mloader/cache/";
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

const char* MLoaderGetErrorMessage()
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
		{ AppStatus::DownloadQueued,	"Download Queued"	},
		{ AppStatus::Downloading, 		"Downloading" 		},
		{ AppStatus::DownloadError, 	"Download Error"	},
		{ AppStatus::Extracting, 		"Extracting"		},
		{ AppStatus::ExtractingError,	"Extracting Error"	},
		{ AppStatus::Downloaded,		"Downloaded"		},
		{ AppStatus::InstallQueued,		"Install Queued"	},
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
		VrpApp* updatedApp = nullptr;
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

	try
	{
		if (!fs::is_directory(cacheDir))
		{
			fs::create_directories(cacheDir);
		}

		if (!fs::is_directory(downloadDir))
		{
			fs::create_directories(downloadDir);
		}
	}
	catch(const fs::filesystem_error& err)
	{
		err_msg = err.what();
		return nullptr;
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

	appContext->QueueManager = new mloader::QueueManager(*appContext->VrpManager, *appContext->Adb, *appContext->Logger);

	try
	{
		GenericCallback(callback, "Loading metadata");
		if (!RefreshMetadata(appContext))
		{
			throw std::runtime_error("Unable to load metadata");
		}
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
			free((char*)context->AppList[i]->Note);

			context->AppList[i]->GameName		= NULL;
			context->AppList[i]->ReleaseName	= NULL;
			context->AppList[i]->PackageName	= NULL;
			context->AppList[i]->LastUpdated	= NULL;
			context->AppList[i]->StatusCStr		= NULL;
			context->AppList[i]->Note			= NULL;

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
		delete context->QueueManager;
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

bool RefreshMetadata(AppContext* context)
{
	context->QueueManager->ClearDownloadQueue();
	context->QueueManager->ClearInstallQueue();
	if (!context->VrpManager->RefreshMetadata())
	{
		err_msg = "Unable to download or load metadata";
		return false;
	}

	return true;
}

void RefreshMetadataAsync(RefreshMetadataAsyncCompletedCallback completedCallback, RefreshMetadataAsyncFailedCallback failedCallback, AppContext* context)
{
	std::thread([=]() {
		std::future<bool> ret = std::async(std::launch::async, RefreshMetadata, context);

		if (ret.get())
		{
			completedCallback(context);
		}
		else
		{
			failedCallback(context);
		}

	}).detach();
}

VrpApp** GetAppList(AppContext* context, int* num)
{
	if (context->AppList == nullptr)	// lazy load
	{
		const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();
		*num = gameInfo.size();
		context->AppList = new VrpApp*[*num];
		int i = 0;
		for (const auto& pair : gameInfo)
		{
			context->AppList[i] = new VrpApp();
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
			context->AppList[i]->StatusCStr 	= GetUpdateStatusString(context->AppList[i]->Status);
			context->AppList[i]->Note			= strdup(context->VrpManager->GetAppNote(pair.first).c_str());
			++i;
		}
	}

	return context->AppList;
}

int DownloadApp(AppContext* context, VrpApp* app)
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

	context->QueueManager->QueueDownload(&it->first);
	return true;
}

int MLoaderInstallApp(AppContext* context, VrpApp* app, AdbDevice* device)
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

	context->QueueManager->QueueInstall(&it->first);

	return true;
}

void MLoaderDeleteApp(AppContext* context, VrpApp* app)
{
	const std::map<mloader::GameInfo, AppStatus>& gameInfo = context->VrpManager->GetGameList();

	auto it = std::find_if(gameInfo.cbegin(), gameInfo.cend(), [app](const auto& pair)
	{
		return pair.first.ReleaseName == app->ReleaseName;
	});

	if (it == gameInfo.end())
	{
		return;
	}

	context->VrpManager->DeleteGame(it->first);
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

char* MLoaderGetDeviceProperty(AppContext* context, AdbDevice* device, const char* propertyName)
{
	if (device == NULL)
	{
		return strdup("");
	}

	const std::string propValue = context->Adb->GetDeviceProperty(*device, propertyName);
	return strdup(propValue.c_str());
}

void MLoaderSetSelectedAdbDevice(AppContext* context, AdbDevice* device)
{
	context->QueueManager->SetSelectedAdbDevice(device);
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

char* GetAppThumbImage(AppContext* context, VrpApp* app)
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

char* MLoaderGetLibraryVersion()
{
	constexpr const int version_size = 10;
	char* version = (char*)malloc(version_size * sizeof(char));
	snprintf(version, version_size, "%s.%s.%s.", MLOADER_VERSION_MAJOR, MLOADER_VERSION_MINOR, MLOADER_VERSION_PATCH);
	return version;
}
