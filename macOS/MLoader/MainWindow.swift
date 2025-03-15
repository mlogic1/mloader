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

// some resources:
// https://developer.apple.com/design/human-interface-guidelines/components

import SwiftUI
import Foundation
import AppKit

struct MainWindow: View {
	@State private var searchFilter: String = ""
	@State private var showFAQ: Bool = false
	
	private var filteredApps: [SVrpApp]
	{
		if searchFilter.isEmpty{
			return coordinator.vrpApps
		}else{
			return coordinator.vrpApps.filter{$0.GameName.lowercased().contains(searchFilter.lowercased())}
		}
	}
	
	var selectDeviceLabel: String{
		if coordinator.selectedAdbDevice == nil{
			return "Select a device"
		}else{
			return coordinator.selectedAdbDevice!.DeviceStatusStr
		}
	}
	
	private var disableInstallButton: Bool{
		if coordinator.selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == coordinator.selectedVrpApp }) {
				return !(selectedVrpApp.Status == Downloaded && coordinator.selectedAdbDevice?.DeviceStatus == OK)
			} else {
				return true
			}
		}
	}
	
	private var disableDownloadButton: Bool{
		if coordinator.selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == coordinator.selectedVrpApp }) {
				return  !(selectedVrpApp.Status == NoInfo)
			}
			return true
		}
	}
	
	@StateObject var coordinator: MainWindowCoordinator = MainWindowCoordinator()
	
	@State var currentPreviewImage: NSImage? = NSImage(named: "ui_thumb_preview") ?? nil
	@State var currentNote: String = ""
	
    var body: some View {
		if coordinator.mLoaderInitialized {
			VStack {
				HStack{
					Menu(selectDeviceLabel){
						ForEach(coordinator.adbDevices) { device in
							Button(device.DeviceStatusStr){
								coordinator.SetSelectedAdbDevice(selectedDevice: device)
							}
						}
					}
					.frame(width: 320.0)
					Button(action: ShowFAQDialog){
						Image(systemName: "questionmark.circle")
					}
					.sheet(isPresented: $showFAQ){
						FAQDialogView()
					}
					Spacer(minLength: 100.0)
					TextField("Search", text: $searchFilter)
						.disableAutocorrection(true)
						.border(.primary)
						.frame(maxWidth: 200.0)
					Button(action: InstallApp){
						Text("Install")
					}
					.disabled(disableInstallButton)
					Button(action: DownloadApp){
						Text("Download")
					}
					.disabled(disableDownloadButton)
				}
				HStack(){
					Menu{
						Button("List View", action: OnViewTypeChanged)
						Button("Tile View", action: OnViewTypeChanged)
					} label: {
						Label("List View", systemImage: "ellipsis.circle")
					}
					.disabled(true)
					.frame(width: 110.0)
				}
				.frame(maxWidth: .infinity, alignment: .trailing)				
				Table(filteredApps, selection: $coordinator.selectedVrpApp){
					TableColumn("Name", value: \.GameName)
						.width(min: 220, ideal: 220, max: 350)
					TableColumn("Status", value: \.StatusStr)
						.width(min: 120, ideal: 155, max: 175)
					TableColumn("Release Name", value: \.ReleaseName)
					TableColumn("Package Name", value: \.PackageName)
					TableColumn("Version") { vrpApp in
						let str: String = String(vrpApp.VersionCode)
						Text(str)
					}
					TableColumn("Last Updated", value: \.LastUpdated)
					TableColumn("Size (MB)") { vrpApp in
						let str: String = String(vrpApp.SizeMB)
						Text(str)
					}
				}
				.onChange(of: coordinator.selectedVrpApp){ selectedAppId in
					UpdatePreviewImage()
				}
				.frame(minWidth: 750, minHeight: 250)
				AppDeviceTabView(previewImage: $currentPreviewImage, noteStr: $currentNote, deviceDetails: $coordinator.selectedAdbDeviceDetails)
			}
			.onReceive(NotificationCenter.default.publisher(for: NSApplication.willTerminateNotification), perform: { output in
					// Code to run on will terminate
					OnApplicationExit()
				})
			.padding()
			.frame(maxWidth: .infinity)
		}
		else{
			ZStack(alignment: .bottomTrailing){
				Image("ui_splash_image")
				Text(coordinator.initStatus)
					.foregroundColor(Color.black)
					.padding(.horizontal, 20)
			}
			.onAppear{
				coordinator.MLoaderInitialize()
			}
			.alert("MLoader failed to start", isPresented: $coordinator.mLoaderInitializationFailed){
				Button("Ok", role: .cancel) {
					NSApplication.shared.terminate(nil)
				}
			} message: {
				Text(coordinator.GetMLoaderLastErrorMessage())
			}
		}
	}
	
	private func UpdatePreviewImage()
	{
		if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == coordinator.selectedVrpApp }) {
			currentPreviewImage = selectedVrpApp.previewImage
			currentNote = selectedVrpApp.Note
		}
	}
	
	private func ShowFAQDialog()
	{
		showFAQ = true
	}
	
	func OnApplicationExit()
	{
		coordinator.MLoaderDestroy()
	}
	
	func InstallApp()
	{
		coordinator.MloaderInstalldAppToSelectedDevice(appId: coordinator.selectedVrpApp!)
	}
				
	func DownloadApp()
	{
		coordinator.MLoaderDownloadApp(appId: coordinator.selectedVrpApp!)
	}
}

struct MainWindow_Previews: PreviewProvider {
    static var previews: some View {
        MainWindow()
    }
}

func OnViewTypeChanged()
{
	print("View type change: Unimplemented")
}
