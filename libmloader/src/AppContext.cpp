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
	mloader::VRPManager*	VrpManager;
	mloader::RClone*		Rclone;
	mloader::Zip*			zip7;
	mloader::ADB*			Adb;
	
	// callbacks
	ADBDeviceListChangedCallback	AdbDeviceListChangedCallback = nullptr;
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

void OnAdbDeviceListChangedEvent(AppContext* context)
{
	if (context->AdbDeviceListChangedCallback)
	{
		context->AdbDeviceListChangedCallback(context);
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
		appContext->VrpManager = new mloader::VRPManager(*appContext->Rclone, *appContext->zip7, cacheDir, downloadDir);
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

void DestroyLoaderContext(AppContext* handle)
{
	if (handle != nullptr)
	{
		delete handle->VrpManager;
		delete handle->Rclone;
		delete handle;
		handle = nullptr;
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

void GetAppList(AppContext* context, App** app, int* num)
{
	const std::vector<mloader::GameInfo>& gameInfo = context->VrpManager->GetGameList();
	size_t numGames = gameInfo.size();
	*num = numGames;
	if (numGames == 0)
	{
		return;
	}

	App* appData = new App[numGames];
	for (int i = 0; i < numGames; ++i)
	{
		appData[i].GameName = gameInfo[i].GameName.c_str();
		appData[i].ReleaseName = gameInfo[i].ReleaseName.c_str();
		appData[i].PackageName = gameInfo[i].PackageName.c_str();
		appData[i].VersionCode = gameInfo[i].VersionCode;
		appData[i].LastUpdated = gameInfo[i].LastUpdated.c_str();
		appData[i].SizeMB = gameInfo[i].SizeMB;
		appData[i].Downloads = gameInfo[i].Downloads;
		appData[i].Rating = gameInfo[i].Rating;
		appData[i].RatingCount = gameInfo[i].RatingCount;
	}

	*app = appData;
}

void FreeAppList(App* app)
{
	delete[] app;
}

bool DownloadApp(AppContext* context, App* app)
{
	const std::vector<mloader::GameInfo>& gameInfo = context->VrpManager->GetGameList();

	auto it = std::find_if(gameInfo.cbegin(), gameInfo.cend(), [app](const mloader::GameInfo& game)
	{
		return game.ReleaseName == app->ReleaseName;
	});

	if (it == gameInfo.end())
	{
		return false;
	}

	context->VrpManager->DownloadGame(*it);
	
	return true;
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

void FreeDeviceList(AdbDevice*& device, int* num)
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
