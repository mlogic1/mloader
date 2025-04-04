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

	@ObservedObject var commandsMenuState: CommandsMenuState
	@ObservedObject var coordinator: MainWindowCoordinator

	var mainWindow: NSWindow?

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

	private var disableDownloadButton: Bool {
		if selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == selectedVrpApp }) {
				return  !(selectedVrpApp.Status == NoInfo)
			}
			return true
		}
	}
	
	private var disableDeleteButton: Bool {
		if selectedVrpApp == nil{
			return true
		}else{
			if let selectedVrpApp = coordinator.vrpApps.first(where: { $0.id == selectedVrpApp }) {
				return  !(selectedVrpApp.Status == Downloaded || selectedVrpApp.Status == Installed)
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
					Spacer(minLength: 30.0)
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
						Button(action: {
							DeleteApp()
						}){
							Text("Delete")
								.foregroundColor(.red)
						}
						.disabled(disableDeleteButton)
					}
				}
				HStack(){
					Menu{
						Button("List View"){
							OnViewTypeChanged(showListViewType: true)
						}
						Button("Grid View"){
							OnViewTypeChanged(showListViewType: false)
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
						},
						onButtonDeleteAction: { vrpApp in
							coordinator.MLoaderDeleteVrpApp(appId: vrpApp.id)
						})
						.frame(minWidth: 750, minHeight: 350)
				}

				AppDeviceTabView(previewImage: $currentPreviewImage, noteStr: $currentNote, deviceDetails: $coordinator.selectedAdbDeviceDetails, showAppDetails: $showListView)
			}
			.onAppear{
				SetWindowPropertiesOnSuccessInitialize()
			}
			.onReceive(NotificationCenter.default.publisher(for: NSApplication.willTerminateNotification), perform: { output in
					// Code to run on will terminate
					OnApplicationExit()
				})
			.padding()
			.frame(maxWidth: .infinity)
			.alert(isPresented: $commandsMenuState.showDeleteAllDownloadsPrompt){
				Alert(title: Text("Clear downloads directory?"), message: Text("This will delete all downloaded applications from this computer. It will not uninstall any applications from connected devices.\nWould you like to continue?"), primaryButton: .default(Text("Yes"), action: {
						coordinator.MLoaderClearDownloadsDirectory()
				}), secondaryButton: .cancel(Text("No")))
			}
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

	private func SetWindowPropertiesOnSuccessInitialize()
	{
		if let window = NSApplication.shared.windows.first {
			  window.styleMask.insert(.resizable)
			  window.setContentSize(NSSize(width: 950, height: 600))
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

	private func OnApplicationExit()
	{
		coordinator.MLoaderDestroy()
	}

	private func InstallApp()
	{
		if let vrpApp = selectedVrpApp{
			coordinator.MloaderInstalldAppToSelectedDevice(appId: vrpApp)
		}
	}
	
	private func DeleteApp()
	{
		if let vrpApp = selectedVrpApp{
			coordinator.MLoaderDeleteVrpApp(appId: vrpApp)
		}
	}

	private func DownloadApp()
	{
		if let vrpApp = selectedVrpApp{
			coordinator.MLoaderDownloadApp(appId: vrpApp)
		}
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
	@StateObject static var commandsMenuState = CommandsMenuState()
	@StateObject static var coordinator: MainWindowCoordinator = MainWindowCoordinator()

    static var previews: some View {
        MainWindow(commandsMenuState: commandsMenuState,
				   coordinator: coordinator)
    }
}
