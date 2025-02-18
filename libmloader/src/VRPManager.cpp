#include "VRPManager.h"
#include "RClone.h"
#include "7z.h"
#include "Logger.h"
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
	VRPManager::VRPManager(const RClone& rclone, const Zip& zip, const fs::path& cacheDir, const fs::path& downloadDir, Logger& logger, std::function<void(const GameInfo&, const AppStatus, const int)> gameStatusChangedCallback)
	:	m_rClone(rclone),
		m_zip(zip),
		m_cacheDir(cacheDir),
		m_downloadDir(downloadDir),
		m_logger(logger),
		m_gameStatusChangedCallback(gameStatusChangedCallback)
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
		m_gameStatusChangedCallback = nullptr;
	}

	bool VRPManager::DownloadMetadata()
	{
		return m_rClone.SyncFile(m_baseUri, "meta.7z", m_cacheDir);
	}

	void VRPManager::ChangeGameStatus(const GameInfo& gameInfo, AppStatus newStatus, int statusParam)
	{
		m_gameList[gameInfo] = newStatus;
		if (m_gameStatusChangedCallback)
		{
			m_gameStatusChangedCallback(gameInfo, newStatus, statusParam);
		}
	}

	bool VRPManager::RefreshMetadata(bool forceRedownload)
	{
		m_logger.LogInfo(LOG_NAME, "Refreshing metdata");
		fs::path metaFile = m_cacheDir / "meta.7z";	
		fs::path metaDir = m_cacheDir / "metadata";
		fs::path gameListFile = metaDir / "VRP-GameList.txt";

		if (!fs::exists(metaFile) || forceRedownload)
		{
			if (!DownloadMetadata())
			{
				return false;
			}
		}
		else
		{
			// TO implement: Check if the metafile is older than 24 hours, then redownload meta regardless
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

		std::ifstream file(gameListFile);

		if (!file.is_open())
		{
			return false;
		}		
		
		std::vector<std::string> csvRows;
		std::string line;

		std::getline(file, line);

		while(std::getline(file, line))
		{
			csvRows.push_back(std::move(line));
		}

		for(const std::string& line : csvRows)
		{
			std::stringstream ss(line);
			std::string item;
			GameInfo info;

			std::getline(ss, item, ';');
			info.GameName = std::move(item);

			std::getline(ss, item, ';');
			info.ReleaseName = std::move(item);

			std::getline(ss, item, ';');
			info.PackageName = std::move(item);
			
			std::getline(ss, item, ';');
			info.VersionCode = std::stoi(item);
		
			std::getline(ss, item, ';');
			info.LastUpdated = std::move(item);
		
			std::getline(ss, item, ';');
			info.SizeMB = std::stoi(item);;

			std::getline(ss, item, ';');
			info.Downloads = std::stof(item);

			std::getline(ss, item, ';');
			info.Rating = std::stof(item);;
			
			std::getline(ss, item, ';');
			info.RatingCount = std::stoi(item);

			// const std::string gameHash = CalculateGameMD5Hash(info.ReleaseName);

			AppStatus appStatus = AppStatus::NoInfo;
			if (GameInstalled(info))
			{
				appStatus = AppStatus::Downloaded;
			}
			m_gameList.emplace(std::move(info), appStatus);
		}
		
		m_logger.LogInfo(LOG_NAME, "Loaded " + std::to_string(m_gameList.size()) + " games from the meta file");
		return true;
	}

	const std::map<GameInfo, AppStatus>& VRPManager::GetGameList() const
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

	void VRPManager::DownloadGame(const GameInfo& game)
	{
		m_logger.LogInfo(LOG_NAME, "Starting download: " + std::string(game.ReleaseName));
		if (m_gameList[game] != AppStatus::NoInfo && m_gameList[game] != AppStatus::DownloadError)
		{
			return; // or throw
		}

		ChangeGameStatus(game, AppStatus::Downloading, 0);

		// Download progress callback
		auto downloadProgressCallbackFunc = [this, game](uint8_t progress) -> void
		{
			if (this->m_gameStatusChangedCallback)
			{
				ChangeGameStatus(game, AppStatus::Downloading, static_cast<int>(progress));
			}
		};

		const std::string gameHash = CalculateGameMD5Hash(game.ReleaseName);
		if (m_rClone.CopyFile(m_baseUri, gameHash, m_cacheDir, downloadProgressCallbackFunc))
		{
			ChangeGameStatus(game, AppStatus::Extracting);
			// find the first .7z file in the temp download dir
			fs::path zippedDirectory = m_cacheDir / fs::path(gameHash);
			fs::path zipFile = findFirstFileWithExtension(zippedDirectory, ".001");

			if (zipFile.empty())
			{
				ChangeGameStatus(game, AppStatus::ExtractingError);
				std::string errMessage = "Unable to locate zip to extract " + std::string(game.ReleaseName);
				m_logger.LogError(LOG_NAME, errMessage);
				throw std::runtime_error(errMessage);
			}
			if (m_zip.Unzip7z(zipFile, m_downloadDir, m_password))
			{
				ChangeGameStatus(game, AppStatus::Downloaded);
			}
			else
			{
				ChangeGameStatus(game, AppStatus::ExtractingError);
			}
		}
		else
		{
			ChangeGameStatus(game, AppStatus::DownloadError);
		}
	}

	std::string VRPManager::GetAppThumbImage(const GameInfo& game) const
	{
		const fs::path metaDir = m_cacheDir / "metadata/.meta/thumbnails/";

		if (!fs::exists(metaDir))
		{
			return "";
		}

		const fs::path thumbFile = metaDir / (game.PackageName + ".jpg");

		if (fs::exists(thumbFile))
		{
			return thumbFile;
		}

		return "";
	}

	bool VRPManager::GameInstalled(const GameInfo& game) const
	{
		const fs::path manifestFile = m_downloadDir / game.ReleaseName / "release.manifest";

		return fs::exists(manifestFile);
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
