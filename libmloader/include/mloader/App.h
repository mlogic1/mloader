#ifndef APP_H
#define APP_H

enum AppStatus
{
	NoInfo = 0,	// default state (downloadable)
	Downloading,
	DownloadError,
	Extracting,
	ExtractingError,
	Installable,
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
} App;


#endif // APP_H
