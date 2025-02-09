#include "AdbDeviceImpl.h"
#include <string>
#include <string.h>
#include <stdlib.h>

bool operator==(const AdbDevice& lhs, const AdbDevice& rhs)
{
	return strcmp(lhs.DeviceId, rhs.DeviceId) == 0 && strcmp(lhs.Model, lhs.Model) == 0 && lhs.DeviceStatus == rhs.DeviceStatus;
}

AdbDevice* CreateAdbDevice(const char* deviceId, const char* model, AdbDeviceStatus deviceStatus)
{
	AdbDevice* device = new AdbDevice();
	device->DeviceId = strdup(deviceId);
	device->Model = strdup(model);
	device->DeviceStatus = deviceStatus;
	return device;
}

AdbDevice* DuplicateAdbDevice(AdbDevice* device)
{
	AdbDevice* duplicateDevice = new AdbDevice();
	
	duplicateDevice->DeviceId = strdup(device->DeviceId);
	duplicateDevice->Model = strdup(device->Model);
	duplicateDevice->DeviceStatus = device->DeviceStatus;
	
	return duplicateDevice;
}

void DestroyAdbDevice(AdbDevice device)
{
	free((char*)device.DeviceId);
	free((char*)device.Model);
}

void DestroyAdbDevice(AdbDevice* device)
{
	DestroyAdbDevice(*device);
	delete device;
	device = nullptr;
}
