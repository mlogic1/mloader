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

var mLoaderAppContextPtr: OpaquePointer? = nil

class MLoaderInitializationManager : ObservableObject{
	@Published var initStatus = ""
}

var mainWindowCoordinatorRef : MainWindowCoordinator? = nil

@_cdecl("MLoaderInitializeCallbackFunction")
private func createLoaderContextStatusCallback(status: UnsafePointer<CChar>?) {
	if let status = status {
		if let statusString = String(validatingUTF8: status){
			DispatchQueue.main.async {
				mainWindowCoordinatorRef?.initStatus = statusString
			}
		}
	}
}

func InitializeGlobalMloaderAsync(onSuccess: @escaping () -> Void, onFailure: @escaping () -> Void) -> Void
{
	var error : Bool = false
	DispatchQueue.global(qos: .userInitiated).async {
		
		// Initialize loader
		// Convert Swift strings to C strings
		let emptyStr = "" // Replace with actual value if needed
		let emptyStrCStr = emptyStr.cString(using: .utf8)
		
		// let result = cFunMLoaderCreateLoaderContext(createLoaderContextStatusCallback, emptyStrCStr, emptyStrCStr)
		mLoaderAppContextPtr = CreateLoaderContext(createLoaderContextStatusCallback, emptyStrCStr, emptyStrCStr)

		if mLoaderAppContextPtr != nil {
			print("AppContext created successfully")
		} else {
			print("Failed to create AppContext")
			error = true
		}
		
		DispatchQueue.main.async {
			if (error){
				onFailure()
			}else{
				onSuccess()
			}
		}
	}
}

func CleanupGlobalMloader()
{
	if (mLoaderAppContextPtr != nil){
		DestroyLoaderContext(mLoaderAppContextPtr)
		mLoaderAppContextPtr = nil
	}
}
