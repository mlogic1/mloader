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
	@State private var showListView: Bool = true
	@State private var viewTypeText: String = "List View"
	@State var selectedVrpApp: SVrpApp.ID? = nil
	@State var currentPreviewImage: NSImage? = NSImage(named: "ui_thumb_preview") ?? nil
	@State var currentNote: String = ""
	
	@StateObject var coordinator: MainWindowCoordinator = MainWindowCoordinator()
	
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
		if selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == selectedVrpApp }) {
				return !(selectedVrpApp.Status == Downloaded && coordinator.selectedAdbDevice?.DeviceStatus == OK)
			} else {
				return true
			}
		}
	}
	
	private var disableDownloadButton: Bool{
		if selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == selectedVrpApp }) {
				return  !(selectedVrpApp.Status == NoInfo)
			}
			return true
		}
	}
	
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
					if showListView{
						Button(action: InstallApp){
							Text("Install")
						}
						.disabled(disableInstallButton)
						Button(action: DownloadApp){
							Text("Download")
						}
						.disabled(disableDownloadButton)
					}
				}
				HStack(){
					Menu{
						Button("List View"){
							OnViewTypeChanged(showListViewType: true)
						}
						Button("Grid View"){
							OnViewTypeChanged(showListViewType:false)
						}
					} label: {
						Label(viewTypeText, systemImage: "ellipsis.circle")
					}
					.frame(width: 110.0)
				}
				.frame(maxWidth: .infinity, alignment: .trailing)
				if showListView {
					VrpAppsListView(selectedVrpApp: $selectedVrpApp, vrpApps: filteredApps)
						.frame(minWidth: 750, minHeight: 350)
						.onChange(of: selectedVrpApp) { selectedAppId in
							UpdatePreviewImage()
						}
				}else{
					VrpAppsGridView(vrpApps: filteredApps, selectedAdbDevice: $coordinator.selectedAdbDevice,
						onButtonDownloadAction: { vrpApp in
							coordinator.MLoaderDownloadApp(appId: vrpApp.id)
						},
						onButtonInstallAction: { vrpApp in
							coordinator.MloaderInstalldAppToSelectedDevice(appId: vrpApp.id)
						})
						.frame(minWidth: 750, minHeight: 350)
				}

				AppDeviceTabView(previewImage: $currentPreviewImage, noteStr: $currentNote, deviceDetails: $coordinator.selectedAdbDeviceDetails, showAppDetails: $showListView)
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
		if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == selectedVrpApp }) {
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
		coordinator.MloaderInstalldAppToSelectedDevice(appId: selectedVrpApp!)
	}
				
	func DownloadApp()
	{
		coordinator.MLoaderDownloadApp(appId: selectedVrpApp!)
	}
	
	private func OnViewTypeChanged(showListViewType: Bool)
	{
		showListView = showListViewType
		if showListView{
			viewTypeText = "List View"
		}else{
			viewTypeText = "Grid View"
		}
	}
}

struct MainWindow_Previews: PreviewProvider {
    static var previews: some View {
        MainWindow()
    }
}
