#include <mloader/AppContext.h>
#include "VRPManager.h"
#include "RClone.h"
#include "curl_global.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace mloader
{
	struct AppContext
	{
		VRPManager* VrpManager;
		RClone*     Rclone;
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

	AppContext* CreateLoaderContext(const char* customCacheDir, const char* customDownloadDir)
	{
		InitGlobalCurl();
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
			if (!fs::create_directory(cacheDir))
			{
				err_msg = "Unable to create cache directory.";
				return NULL;
			}
		}

		if (!fs::exists(downloadDir))
		{
			if (!fs::create_directory(downloadDir))
			{
				err_msg = "Unable to create download directory.";
				return NULL;
			}
		}


		AppContext* appContext = new AppContext();

		try
		{
			appContext->Rclone = new RClone(cacheDir);
		}
		catch(std::runtime_error& error)
		{
			err_msg = error.what();
			delete appContext;
			return nullptr;
		}

		try
		{
			appContext->VrpManager = new VRPManager(*appContext->Rclone, cacheDir, downloadDir);
		}
		catch(std::runtime_error& error)
		{
			err_msg = error.what();
			delete appContext->Rclone;
			delete appContext;
			return nullptr;
		}

		return appContext;
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
		CleanupGlobalCurl();
	}

	void RefreshMetadata(AppContext* context)
	{
		// context->VrpManager->RefreshMeta();

		context->VrpManager->DbgRefreshMeta();
	}
	
}
