#include <mloader/AppContext.h>
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

namespace fs = std::filesystem;

struct AppContext
{
	mloader::VRPManager*			VrpManager;
	mloader::RClone*				Rclone;
	mloader::Zip*					zip7;
	mloader::ADB*					Adb;

	// App list
	App** 							AppList = nullptr;
	// TODO: make device list context owned as well
	
	// callbacks
	ADBDeviceListChangedCallback	AdbDeviceListChangedCallback		= nullptr;
	AppStatusChangedCallback		AppsStatusChangedCallback			= nullptr;
	void*							AppsStatusChangedCallbackUserData	= nullptr;
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
		{ AppStatus::Installing,		"Installing"		}
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
	if (context->AdbDeviceListChangedCallback)
	{
		context->AdbDeviceListChangedCallback(context);
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
		GenericCallback(callback, "Initializing RClone");
		appContext->Rclone = new mloader::RClone(cacheDir);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Initializing 7-Zip");
		appContext->zip7 = new mloader::Zip(cacheDir);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext;
		return nullptr;
	}

	try
	{
		GenericCallback(callback, "Initializing ADB");
		appContext->Adb = new mloader::ADB(cacheDir, std::bind(OnAdbDeviceListChangedEvent, appContext));
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->zip7;
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

		appContext->VrpManager = new mloader::VRPManager(*appContext->Rclone, *appContext->zip7, cacheDir, downloadDir, onAppStatusChanged);
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete appContext->Rclone;
		delete appContext->zip7;
		delete appContext->Adb;
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
		delete appContext->zip7;
		delete appContext->Adb;
		delete appContext->VrpManager;
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

			context->AppList[i]->GameName 	= NULL;
			context->AppList[i]->ReleaseName = NULL;
			context->AppList[i]->PackageName = NULL;
			context->AppList[i]->LastUpdated = NULL;
			context->AppList[i]->StatusCStr = NULL;

			delete context->AppList[i];
		}
		delete[] context->AppList;
	}

	context->AdbDeviceListChangedCallback		= nullptr;
	context->AppsStatusChangedCallback			= nullptr;
	context->AppsStatusChangedCallbackUserData	= nullptr;

	if (context != nullptr)
	{
		delete context->VrpManager;
		delete context->Rclone;
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

App** GetAppList(AppContext* context, App** &app, int* num)
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

void GetDeviceList(AppContext* context, AdbDevice*& device, int* num)
{
	std::vector<AdbDevice> adbDevices = context->Adb->GetAdbDevices();
	*num = static_cast<int>(adbDevices.size());
	device = new AdbDevice[*num];
	
	for(int i = 0; i < *num; ++i)
	{
		device[i] = *DuplicateAdbDevice(&adbDevices[i]);
	}
}

void FreeDeviceList(AdbDevice* device, int* num)
{
	for (int i = 0; i < *num ; ++i)
	{
		DestroyAdbDevice(device[i]);
		delete &device[i];
		device = nullptr;
	}
}

void SetADBDeviceListChangedCallback(AppContext* context, ADBDeviceListChangedCallback callback)
{
	context->AdbDeviceListChangedCallback = callback;
}

void ClearADBDeviceListChangedCallback(AppContext* context)
{
	context->AdbDeviceListChangedCallback = nullptr;
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
