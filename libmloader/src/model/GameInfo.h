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

	bool operator<(const GameInfo& lhs, const GameInfo& rhs);
}

#endif // GAMEINFO_H