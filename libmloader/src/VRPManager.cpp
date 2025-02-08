#include "VRPManager.h"
#include "RClone.h"
#include "7z.h"
#include "md5.h"
#include <filesystem>
#include "curl_global.h"
#include <curl/curl.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

namespace mloader
{
	VRPManager::VRPManager(const RClone& rclone, const Zip& zip, const fs::path& cacheDir, const fs::path& downloadDir)
	:	m_rClone(rclone),
		m_zip(zip),
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
		m_rClone.SyncFile(m_baseUri, "meta.7z", m_cacheDir);
		return true;
	}

	bool VRPManager::DbgRefreshMeta()
	{
		fs::path metaFile = m_cacheDir / "meta.7z";	
		fs::path metaDir = m_cacheDir / "metadata";
		fs::path gameListFile = metaDir / "VRP-GameList.txt";

		if (!fs::exists(metaFile))
		{
			return false;
		}

		fs::remove_all(metaDir);
		fs::create_directories(metaDir);

		if (!m_zip.Unzip7z(metaFile, metaDir, m_password))
		{
			return false;
		}

		if (!fs::exists(gameListFile))
		{
			return false;
		}

		// refresh game list
		m_gameList.clear();
		m_gameList.reserve(4096);

		std::ifstream file(gameListFile);

		if (!file.is_open())
		{
			return false;
		}		

		std::string line;

		std::getline(file, line);

		while(std::getline(file, line))
		{
			std::stringstream ss(line);
			std::string item;
			GameInfo info;

			std::getline(ss, item, ';');
			info.GameName = item;

			std::getline(ss, item, ';');
			info.ReleaseName = item;

			std::getline(ss, item, ';');
			info.PackageName = item;
			
			std::getline(ss, item, ';');
			info.VersionCode = std::stoi(item);
		
			std::getline(ss, item, ';');
			std::string LastUpdated;
		
			std::getline(ss, item, ';');
			info.SizeMB = std::stoi(item);;

			std::getline(ss, item, ';');
			info.Downloads = std::stof(item);

			std::getline(ss, item, ';');
			info.Rating = std::stof(item);;
			
			std::getline(ss, item, ';');
			info.RatingCount = std::stoi(item);

			const std::string gameHash = CalculateGameMD5Hash(info.ReleaseName);

			m_gameList.push_back(std::move(info));
		}
		return true;
	}

	const std::vector<GameInfo>& VRPManager::GetGameList() const
	{
		return m_gameList;
	}


	static fs::path findFirstFileWithExtension(const fs::path& dirPath, const std::string& extension) {
		for (const auto& entry : fs::directory_iterator(dirPath)) {
			if (entry.is_regular_file() && entry.path().extension() == extension) {
				return entry.path();
			}
		}
		return ""; // Return an empty path if no file is found
	}

	void VRPManager::DownloadGame(const GameInfo& game) const
	{
		const std::string gameHash = CalculateGameMD5Hash(game.ReleaseName);
		if (m_rClone.CopyFile(m_baseUri, gameHash, m_cacheDir))
		{
			// find the first .7z file in the temp download dir
			fs::path zippedDirectory = m_cacheDir / fs::path(gameHash);
			fs::path zipFile = findFirstFileWithExtension(zippedDirectory, ".001");

			if (zipFile.empty())
			{
				throw std::runtime_error("Unable to locate zip to extract " + game.ReleaseName);
			}
			m_zip.Unzip7z(zipFile, m_downloadDir, m_password);
		}
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
