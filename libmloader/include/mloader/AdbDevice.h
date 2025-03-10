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

#ifndef ADB_DEVICE_H
#define ADB_DEVICE_H

#include <string.h>

typedef enum
{
	OK = 0,
	UnAuthorized,
	Offline,
	NoPermissions,
	Unknown
} AdbDeviceStatus;

typedef struct 
{
	const char* DeviceId;
	const char* Model;
	AdbDeviceStatus DeviceStatus;
} AdbDevice;

#endif // ADB_DEVICE_H
