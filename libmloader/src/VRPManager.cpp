#include "VRPManager.h"
#include "RClone.h"
#include <filesystem>
#include "curl_global.h"
#include <curl/curl.h>
#include <exception>
#include <iostream>
#include <fstream>

namespace mloader
{
	VRPManager::VRPManager(const RClone& rclone, const fs::path& cacheDir, const fs::path& downloadDir)
	:	m_rClone(rclone),
		m_cacheDir(cacheDir),
		m_downloadDir(downloadDir)
	{
		std::string error{""};

		if (!CheckVRPPublicCredentials())
		{
			error = "Unable to download vrp-public.json from VRP";
			throw std::runtime_error(error);
		}
	}

	VRPManager::~VRPManager()
	{
		
	}

	bool VRPManager::RefreshMeta()
	{
		m_rClone.SyncFile("https://go.vrpyourself.online/", "meta.7z", m_cacheDir);
		return true;
	}

	bool VRPManager::DbgRefreshMeta()
	{
		fs::path metaFile{"/tmp/mloader/cache/meta.7z"};	
		const std::string password = "gL59VfgPxoHR";

		return true;
	}

	bool VRPManager::CheckVRPPublicCredentials()
	{
		static const std::string vrppublic = "https://vrpirates.wiki/downloads/vrp-public.json";
		static const fs::path filePath = m_cacheDir / "vrp-public.json";

		if (!fs::exists(filePath))
		{
			return CurlDownloadFile(vrppublic, filePath);
		}
		else
		{
			// TODO: check how old the file is.
			// if older than 24 hours download it again
		}
		
		return true;
	}
}
