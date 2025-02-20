#include "AdbDeviceImpl.h"
#include <string>
#include <string.h>
#include <stdlib.h>

bool operator==(const AdbDevice& lhs, const AdbDevice& rhs)
{
	return strcmp(lhs.DeviceId, rhs.DeviceId) == 0 && strcmp(lhs.Model, lhs.Model) == 0 && lhs.DeviceStatus == rhs.DeviceStatus;
}

void DestroyAdbDevice(AdbDevice device)
{
	free((char*)device.DeviceId);
	free((char*)device.Model);
}