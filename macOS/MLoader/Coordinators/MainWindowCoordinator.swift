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
import SwiftUI
import AppKit

var mainWindowCoordinatorRef : MainWindowCoordinator? = nil	// to be fixed, this variable is used during initialization to receive the status messages

class MainWindowCoordinator: ObservableObject{

	@Published var initStatus = ""
	@Published var mLoaderInitialized : Bool = false
	
	// in case of failed initialization
	@Published var mLoaderInitializationFailed : Bool = false
	
	// Use mLoaderAppContextPtr to interact with the library
	private var mLoaderAppContextPtr: OpaquePointer?
	
	@Published var vrpApps: [SVrpApp] =
	[
	]
	
	
	@Published var adbDevices: [SAdbDevice] =
	[
	]
	
	@Published var selectedAdbDevice : SAdbDevice? = nil
	@Published var selectedAdbDeviceDetails: [SAdbDeviceDetails] = [
		SAdbDeviceDetails(PropertyName: "ro.product.manufacturer"),
		SAdbDeviceDetails(PropertyName: "ro.product.model"),
		SAdbDeviceDetails(PropertyName: "ro.serialno"),
		SAdbDeviceDetails(PropertyName: "ro.build.version.release"),
		SAdbDeviceDetails(PropertyName: "ro.build.version.sdk"),
		SAdbDeviceDetails(PropertyName: "ro.build.branch")
	]
	
	init(){
		mainWindowCoordinatorRef = self
	}
	
	private func RefreshAppList()
	{
		vrpApps.removeAll()
		var numApps : Int32 = 0
		var appListPtr : UnsafeMutablePointer<UnsafeMutablePointer<VrpApp>?>! = nil
		appListPtr = GetAppList(mLoaderAppContextPtr, &numApps)
		
		for i in 0..<Int(numApps){
			if let appPointer = appListPtr[i]{
				let vrpApp = appPointer.pointee
				
				var thumbImagePath : String = ""
				if let thumbImageCPath = GetAppThumbImage(mLoaderAppContextPtr, appPointer){
					thumbImagePath = Utility_StringFromCString(thumbImageCPath)
					free(thumbImageCPath)
				}
				
				let sVrpApp = SVrpApp(GameName: Utility_StringFromCString(vrpApp.GameName),
									  ReleaseName: Utility_StringFromCString(vrpApp.ReleaseName),
									  PackageName: Utility_StringFromCString(vrpApp.PackageName),
									  VersionCode: vrpApp.VersionCode,
									  LastUpdated: Utility_StringFromCString(vrpApp.LastUpdated),
									  SizeMB: vrpApp.SizeMB,
									  Downloads: vrpApp.Downloads,
									  Rating: vrpApp.Rating,
									  RatingCount: vrpApp.RatingCount,
									  Status: vrpApp.Status,
									  AppStatusParam: vrpApp.AppStatusParam,
									  StatusStr: Utility_StringFromCString(vrpApp.StatusCStr),
									  Note: Utility_StringFromCString(vrpApp.Note),
									  cAppPtr: appPointer,
									  previewImage: LoadImageFromDisk(imageFile: thumbImagePath))
				vrpApps.append(sVrpApp)
			}
		}
	}
	
	private func SetupCallbackEvents()
	{
		let ptrSelf = Unmanaged.passUnretained(self).toOpaque()
		SetADBDeviceListChangedCallback(mLoaderAppContextPtr, COnAdbDeviceListChanged, ptrSelf)
		
		SetAppStatusChangedCallback(mLoaderAppContextPtr, COnAppStatusChanged, ptrSelf)
	}
	
	private func RefreshDeviceList()
	{
		adbDevices.removeAll()
		var numDevices : Int32 = 0
		
		var deviceListPtr : UnsafeMutablePointer<UnsafeMutablePointer<AdbDevice>?>! = nil
		
		deviceListPtr = GetDeviceList(mLoaderAppContextPtr, &numDevices)
		
		for i in 0..<Int(numDevices){
			if let devicePointer = deviceListPtr[i]{
				let adbDevice = devicePointer.pointee
				let sAdbDevice = SAdbDevice(DeviceId: Utility_StringFromCString(adbDevice.DeviceId),
											Model: Utility_StringFromCString(adbDevice.Model),
											DeviceStatus: adbDevice.DeviceStatus,
											cAdbDevicePtr: devicePointer)

				adbDevices.append(sAdbDevice)
			}
		}
		
		// Select a device by default
		if adbDevices.count > 0 {
			SetSelectedAdbDevice(selectedDevice: adbDevices[0])
		}else{
			SetSelectedAdbDevice(selectedDevice: nil)
		}
	}
	
	private func OnMLoaderInitSucessful(mloaderContext: OpaquePointer?)
	{
		mLoaderInitialized = true
		mLoaderAppContextPtr = mloaderContext
		SetupCallbackEvents()
		RefreshAppList()
		RefreshDeviceList()
	}
	
	func GetMLoaderLastErrorMessage() -> String
	{
		let cStrReason = MLoaderGetErrorMessage()
		let mLoaderInitializationFailedReason = Utility_StringFromCString(cStrReason)
		return mLoaderInitializationFailedReason
	}
	
	private func OnMloaderInitFailed()
	{
		mLoaderInitializationFailed = true
	}
	
	private func LoadImageFromDisk(imageFile: String) -> NSImage? {
		return NSImage(contentsOfFile: imageFile) ?? NSImage(named: "ui_thumb_preview")
	}
	
	private func UpdateSelectedDeviceDetails()
	{
		if selectedAdbDevice == nil{
			for index in selectedAdbDeviceDetails.indices{
				selectedAdbDeviceDetails[index].PropertyValue = ""
			}
		}else{
			for index in selectedAdbDeviceDetails.indices{
				let propNameCStr = selectedAdbDeviceDetails[index].PropertyName.cString(using: .utf8)
				let propValueCStr = MLoaderGetDeviceProperty(mLoaderAppContextPtr, selectedAdbDevice?.cAdbDevicePtr, propNameCStr)
				selectedAdbDeviceDetails[index].PropertyValue = Utility_StringFromCString(propValueCStr)
				free(propValueCStr)
			}
		}
	}
	
	func SetSelectedAdbDevice(selectedDevice: SAdbDevice?){
		selectedAdbDevice = selectedDevice
		MLoaderSetSelectedAdbDevice(mLoaderAppContextPtr, selectedAdbDevice?.cAdbDevicePtr)
		UpdateSelectedDeviceDetails()
	}
	
	func MLoaderDownloadApp(appId: SVrpApp.ID)
	{
		if let app = vrpApps.first(where: { $0.id == appId }){
			DownloadApp(mLoaderAppContextPtr, app.cAppPtr)
		}
	}
	
	func MloaderInstalldAppToSelectedDevice(appId: SVrpApp.ID)
	{
		if let app = vrpApps.first(where: { $0.id == appId }){
			if selectedAdbDevice != nil{
				MLoaderInstallApp(mLoaderAppContextPtr, app.cAppPtr, selectedAdbDevice!.cAdbDevicePtr)
			}
		}
	}
	
	func MLoaderClearDownloadsDirectory()
	{
		for vrpApp in vrpApps{
			if vrpApp.Status == Downloaded{
				MLoaderDeleteApp(mLoaderAppContextPtr, vrpApp.cAppPtr)
			}
		}
	}
	
	func MLoaderInitialize()
	{
		InitializeMloaderAsync(onSuccess: self.OnMLoaderInitSucessful, onFailure: self.OnMloaderInitFailed)
	}
	
	func MLoaderDestroy()
	{
		if (mLoaderAppContextPtr != nil){
			DestroyLoaderContext(mLoaderAppContextPtr)
		}
	}
	
	func OnAdbDeviceListChanged()
	{
		RefreshDeviceList()
	}
	
	func OnAppStatusChanged(app: UnsafeMutablePointer<VrpApp>?)
	{
		if app != nil{
			let appPtr = app?.pointee
			let appReleaseName = Utility_StringFromCString(appPtr?.ReleaseName)
			
			let index : Int? = vrpApps.firstIndex { SVrpApp in
				SVrpApp.ReleaseName == appReleaseName
			}
			
			if index != nil{
				vrpApps[index!].Status = appPtr!.Status
				vrpApps[index!].StatusStr = Utility_StringFromCString(appPtr!.StatusCStr)
				vrpApps[index!].AppStatusParam = appPtr!.AppStatusParam
			}
		}
	}

	private func InitializeMloaderAsync(onSuccess: @escaping (OpaquePointer?) -> Void, onFailure: @escaping () -> Void) -> Void
	{
		var error : Bool = false
		DispatchQueue.global(qos: .userInitiated).async {
			
			// Initialize loader
			// Convert Swift strings to C strings
			let emptyStr = "" // Replace with actual value if needed
			let emptyStrCStr = emptyStr.cString(using: .utf8)
			
			let mLoaderAppContextPtr: OpaquePointer? = CreateLoaderContext(createLoaderContextStatusCallback, emptyStrCStr, emptyStrCStr)

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
					onSuccess(mLoaderAppContextPtr)
				}
			}
		}
	}
}

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

@_cdecl("MLoaderAdbDeviceListChangedCallback")
private func COnAdbDeviceListChanged(appContext: Optional<OpaquePointer>, userData: UnsafeMutableRawPointer!)
{
	DispatchQueue.main.async {
		let instance = Unmanaged<MainWindowCoordinator>.fromOpaque(userData).takeUnretainedValue()
		instance.OnAdbDeviceListChanged()
	}
}

@_cdecl("MLoaderAppStatusChangedCallback")
private func COnAppStatusChanged(appContext: Optional<OpaquePointer>, app: UnsafeMutablePointer<VrpApp>?, userData: UnsafeMutableRawPointer!)
{
	DispatchQueue.main.async {
		let instance = Unmanaged<MainWindowCoordinator>.fromOpaque(userData).takeUnretainedValue()
		
		instance.OnAppStatusChanged(app: app)
	}
}
