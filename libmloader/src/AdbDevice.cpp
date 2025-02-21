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