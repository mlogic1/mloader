#ifndef ADB_DEVICE_PRIV_H
#define ADB_DEVICE_PRIV_H

#include <mloader/AdbDevice.h>
#include <string>

bool operator==(const AdbDevice& lhs, const AdbDevice& rhs);

AdbDevice* CreateAdbDevice(const char* deviceId, const char* model, AdbDeviceStatus deviceStatus);
AdbDevice* DuplicateAdbDevice(AdbDevice* device);
void DestroyAdbDevice(AdbDevice device);
void DestroyAdbDevice(AdbDevice* device);

#endif // ADB_DEVICE_PRIV_H
