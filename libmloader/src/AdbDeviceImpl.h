#ifndef ADB_DEVICE_PRIV_H
#define ADB_DEVICE_PRIV_H

#include <mloader/AdbDevice.h>
#include <string>

bool operator==(const AdbDevice& lhs, const AdbDevice& rhs);
void DestroyAdbDevice(AdbDevice device);

#endif // ADB_DEVICE_PRIV_H
