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

import Foundation

// S(wift)AdbDevice
struct SAdbDevice : Identifiable{
	let id = UUID()
	var DeviceId: String
	var Model: String
	var DeviceStatus: AdbDeviceStatus
	
	// Additionally formatted string, used for UI to show the state
	var DeviceStatusStr: String{
		if DeviceStatus == UnAuthorized{
			return DeviceId + " (Unauthorized)"
		}
		if DeviceStatus == Offline{
			return DeviceId + " (Offline)"
		}
		if DeviceStatus == NoPermissions{
			return DeviceId + " (No Permissions)"
		}
		if DeviceStatus == Unknown{
			return DeviceId + " (Unknown State)"
		}
		return DeviceId
	}
	
	var cAdbDevicePtr: UnsafeMutablePointer<AdbDevice>
}
