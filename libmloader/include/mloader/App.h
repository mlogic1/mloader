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
