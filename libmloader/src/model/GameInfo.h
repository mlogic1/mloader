#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <string>
#include <unistd.h>

namespace mloader
{
	struct GameInfo
	{
		std::string GameName;
		std::string ReleaseName;
		std::string PackageName;
		int32_t VersionCode;
		std::string LastUpdated;
		int32_t SizeMB;
		float Downloads;			// popularity
		float Rating;
		int32_t RatingCount;
	};
}

#endif // GAMEINFO_H