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

#include <mloader/SAContext.h>
#include "ADB.h"
#include "Logger.h"
#include "curl_global.h"
#include <exception>
#include <filesystem>
#include <future>
#include <thread>

namespace fs = std::filesystem;

struct SAContext
{
	mloader::ADB*					Adb;
	mloader::Logger*				Logger;

	AdbDevice**						AdbDeviceList = nullptr;
	AdbDevice* 						SelectedAdbDevice = nullptr;

	// callbacks
	SAADBDeviceListChangedCallback	AdbDeviceListChangedCallback			= nullptr;
	void*							AdbDeviceListChangedCallbackUserData	= nullptr;
};

static const char* err_msg;

const char* MLoaderSAGetErrorMessage()
{
	return err_msg;
}

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

static inline void GenericCallback(CreateLoaderContextStatusCallback callback, const char* message)
{
	if (callback != NULL)
	{
		callback(message);
	}
}

static int RefreshAdbDeviceList(SAContext* context)
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

void OnAdbDeviceListChangedEvent(SAContext* context)
{
	RefreshAdbDeviceList(context);

	if (context->AdbDeviceListChangedCallback)
	{
		context->AdbDeviceListChangedCallback(context, context->AdbDeviceListChangedCallbackUserData);
	}
}

SAContext* MLoaderSACreateLoaderContext(CreateLoaderContextStatusCallback callback, const char* customCacheDir)
{
	GenericCallback(callback, "Initializing");
	mloader::InitGlobalCurl();

	fs::path cacheDir = DetermineCacheDir();

	if (customCacheDir[0] != '\0')
	{
		cacheDir = customCacheDir;
	}

	if (!fs::exists(cacheDir))
	{
		if (!fs::create_directories(cacheDir))
		{
			err_msg = "Unable to create cache directory.";
			return NULL;
		}
	}

	SAContext* saContext = new SAContext();

	try
	{
		saContext->Logger = new mloader::Logger(cacheDir / "mloader.log");
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete saContext;
		return nullptr;
	}

	saContext->Logger->LogInfo("SA Initialization", "Starting up");
	saContext->Logger->LogInfo("SA Initialization", "Cache directory: " + std::string(cacheDir));

	try
	{
		GenericCallback(callback, "Initializing ADB");
		saContext->Adb = new mloader::ADB(cacheDir, *saContext->Logger, std::bind(OnAdbDeviceListChangedEvent, saContext));
	}
	catch(std::runtime_error& error)
	{
		err_msg = error.what();
		delete saContext->Logger;
		delete saContext;
		return nullptr;
	}

	return saContext;
}

void MLoaderSACreateLoaderContextAsync(CreateSALoaderContextAsyncCompletedCallback completedCallback, CreateLoaderContextStatusCallback callback, const char* customCacheDir)
{
	std::thread([=]() {
		std::future<SAContext*> ret = std::async(std::launch::async, MLoaderSACreateLoaderContext, callback, customCacheDir);
		SAContext* result = ret.get();

		completedCallback(result);
	}).detach();
}

void MLoaderSADestroyLoaderContext(SAContext* context)
{
	if (context->AdbDeviceList)
	{
		delete[] context->AdbDeviceList;
		context->AdbDeviceList = nullptr;
	}

	context->AdbDeviceListChangedCallback = nullptr;

	if (context != nullptr)
	{
		delete context->Adb;
		delete context->Logger;
		delete context;
		context = nullptr;
	}
	mloader::CleanupGlobalCurl();
}

bool MLoaderSAInstallFile(SAContext* context, const char* apkFile, AdbDevice* device, const char* obbPackageName)
{
	const std::vector<fs::path> files{ {apkFile} };
	try
	{
		context->Adb->InstallFilesToDevice(obbPackageName, files, *device);
	}catch(std::runtime_error& err)
	{
		err_msg = err.what();
		return false;
	}

	return true;
}

AdbDevice** MLoaderSAGetDeviceList(SAContext* context, int* num)
{
	if (context->AdbDeviceList)
	{
		*num = context->Adb->GetAdbDevices().size();
		return context->AdbDeviceList;
	}

	*num = RefreshAdbDeviceList(context);

	return context->AdbDeviceList;
}

void MLoaderSASetSelectedAdbDevice(SAContext* context, AdbDevice* device)
{
	context->SelectedAdbDevice = device;
}

void MLoaderSASetADBDeviceListChangedCallback(SAContext* context, SAADBDeviceListChangedCallback callback, void* userData)
{
	context->AdbDeviceListChangedCallback = callback;
	context->AdbDeviceListChangedCallbackUserData = userData;
}

void MLoaderSAClearADBDeviceListChangedCallback(SAContext* context)
{
	context->AdbDeviceListChangedCallback = nullptr;
	context->AdbDeviceListChangedCallbackUserData = nullptr;
}
