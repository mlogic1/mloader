#ifndef ADB_DEVICE_H
#define ADB_DEVICE_H

#include <string.h>

enum AdbDeviceStatus
{
	OK = 0,
	UnAuthorized,
	Offline,
	NoPermissions,
	Unknown
};

typedef struct 
{
	const char* DeviceId;
	const char* Model;
	AdbDeviceStatus DeviceStatus;
} AdbDevice;

#endif // ADB_DEVICE_H
