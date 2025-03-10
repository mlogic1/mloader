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

import SwiftUI
import Foundation

struct MainWindow: View {
	
	@State private var mloaderInitialized : Bool = false
	
	@State private var searchFilter: String = ""
	// Use mLoaderAppContextPtr to interact with the library
	
	@State private var vrpApps: [SVrpApp] =
	[
	]
	
	private var filteredApps: [SVrpApp]
	{
		if searchFilter.isEmpty{
			return vrpApps
		}else{
			return vrpApps.filter{$0.GameName.lowercased().contains(searchFilter.lowercased())}
		}
	}
	
	@State var selectedVrpApp: SVrpApp.ID? = nil
	
	@ObservedObject var initManager : MLoaderInitializationManager = MLoaderInitializationManager()
	
	init(){
		globalInitStatus = self.initManager
	}
	
    var body: some View {
		if mloaderInitialized {
			VStack {
				HStack{
					Menu{
						Button("Option 1", action: OnDeviceSelectionChanged)
						Button("Option 2", action: OnDeviceSelectionChanged)
					} label: {
						Label("Select a device", systemImage: "ellipsis.circle")
					}
					.frame(width: 190.0)
					Spacer(minLength: 100.0)
					TextField("Search", text: $searchFilter)
						.disableAutocorrection(true)
						.border(.primary)
						.frame(maxWidth: 200.0)
					Button(action: Install)
					{
						Text("Install")
					}
					Button(action: Download)
					{
						Text("Download")
					}
				}
				HStack(){
					Menu{
						Button("List View", action: OnViewTypeChanged)
						Button("Tile View", action: OnViewTypeChanged)
					} label: {
						Label("List View", systemImage: "ellipsis.circle")
					}
					.frame(width: 110.0)
				}
				.frame(maxWidth: .infinity, alignment: .trailing)
				VSplitView{
					Table(filteredApps, selection: $selectedVrpApp){
						TableColumn("Name", value: \.GameName)
						TableColumn("Status", value: \.StatusStr)
							.width(65)
						TableColumn("Release Name", value: \.ReleaseName)
						TableColumn("Package Name", value: \.PackageName)
						TableColumn("Version") { vrpApp in
							Text("\(vrpApp.VersionCode)")
						}
						TableColumn("Last Updated", value: \.LastUpdated)
						TableColumn("Size (MB)") { vrpApp in
							Text("\(vrpApp.SizeMB)")
						}
					}
					.background(Color.red)
					Circle()	// Replace circle with tab views
						.fill(.yellow)
						.frame(idealHeight: 140.0)
				}
				.background(Color.green)
				.frame(minWidth: 750.0, maxWidth: .infinity, minHeight: 550.0, maxHeight: .infinity)
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
				Image("splash_image")
				Text(initManager.initStatus)
					.foregroundColor(Color.black)
					.padding(.horizontal, 20)
			}
			Button("Press to initialize", action: InitializeBtnAction)
		}
	}
	
	func OnMLoaderInitSucessful()
	{
		mloaderInitialized = true
		
		var numApps : Int32 = 0
		var appListPtr : UnsafeMutablePointer<UnsafeMutablePointer<VrpApp>?>! = nil
		appListPtr = GetAppList(mLoaderAppContextPtr, &numApps)
		
		vrpApps.removeAll()
		
		for i in 0..<Int(numApps){
			if let appPointer = appListPtr[i]{
				let vrpApp = appPointer.pointee
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
									  Note: Utility_StringFromCString(vrpApp.Note))
				vrpApps.append(sVrpApp)
			}
		}
	}
	
	func OnMloaderInitFailed()
	{
		// TODO: display a window why and close the application
	}
	
	func OnApplicationExit()
	{
		CleanupGlobalMloader()
	}
	
	func InitializeBtnAction()
	{
		// TODO: it's still crashing on create_directories
		InitializeGlobalMloaderAsync(onSuccess: self.OnMLoaderInitSucessful, onFailure: self.OnMloaderInitFailed)
	}
	
	func RefreshAppList()
	{
		
	}
}

struct MainWindow_Previews: PreviewProvider {
    static var previews: some View {
        MainWindow()
    }
}

func OnViewTypeChanged()
{
	print("View type change")
}

func Install()
{
	print("Install pressed")
}
			
func Download()
{
	print("Download pressed")
}
			
func OnDeviceSelectionChanged()
{
	print("device changed now")
}
