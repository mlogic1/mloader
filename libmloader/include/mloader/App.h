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

#ifndef APP_H
#define APP_H

enum AppStatus
{
	NoInfo = 0,	// default state (downloadable)
	Downloading,	// TODO: implement queued
	DownloadError,
	Extracting,
	ExtractingError,
	Downloaded,
	Installing,
	InstallingError,
	Installed
};

typedef struct 
{
	const char* GameName;
	const char* ReleaseName;
	const char* PackageName;
	int VersionCode;
	const char* LastUpdated;
	int SizeMB;
	float Downloads;
	float Rating;
	int RatingCount;
	AppStatus Status;
	int AppStatusParam;			// When downloading or extracting, progress is reported with this param, otherwise it defaults to -1
	const char* StatusCStr;		// Formatted Status as string with param, e.g. "Downloading (19%)" or "Extracting (24%)"
} App;

#endif // APP_H
