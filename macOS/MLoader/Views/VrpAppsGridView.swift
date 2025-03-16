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

struct VrpAppsGridView: View {
	var vrpApps: [SVrpApp]
	@Binding var selectedAdbDevice : SAdbDevice?
	var onButtonDownloadAction: ((SVrpApp) -> Void )?
	var onButtonInstallAction: ((SVrpApp) -> Void )?
	
    var body: some View {
		GeometryReader{ geometry in
			let currentWidth = geometry.size.width
			let columnCount = max(1, Int(currentWidth / 262))
			let gridColumns = Array(repeating: GridItem(.fixed(262), spacing: 10, alignment: nil), count: columnCount)
			ScrollView{
				LazyVGrid(columns: gridColumns, spacing: 16){
					ForEach(vrpApps) { vrpApp in
						let strSize: String = "Size: " + String(vrpApp.SizeMB) + " MB"
						let strVersion: String = "Version: " + String(vrpApp.VersionCode)
						VStack{
							Image(nsImage: vrpApp.previewImage!)
								.resizable()
								.scaledToFit()
								.frame(width: 262, height: 150)
								.cornerRadius(4)
							Text(vrpApp.GameName)
								.font(.headline)
								.frame(minWidth: 262, idealWidth: 262, maxWidth: 262, minHeight: 20, idealHeight: 20, maxHeight: 50, alignment: .center)
							Text(strSize)
								.font(.body)
								.foregroundColor(.secondary)
								.padding(0)
								.frame(width: 262, alignment: .leading)
							Text(strVersion)
								.font(.body)
								.foregroundColor(.secondary)
								.padding(0)
								.frame(width: 262, alignment: .leading)
							Text("Last updated: " + vrpApp.LastUpdated)
								.font(.subheadline)
								.foregroundColor(.secondary)
								.padding(0)
								.frame(width: 262, alignment: .trailing)
							Button(action: {
								OnActionButtonClicked(vrpApp: vrpApp)
							}){
								Text(DetermineButtonLabel(vrpApp: vrpApp))
									.font(.body)
									.frame(width: 200)
							}
							.disabled(DetermineButtonDisabled(vrpApp: vrpApp))
							.padding(5)
						}
						.frame(width: 262)
						.background(RoundedRectangle(cornerRadius: 4).fill(Color(red: 0.17, green: 0.16, blue: 0.20)))
					}
				}
			}
		}
    }
	
	private func DetermineButtonLabel(vrpApp: SVrpApp) -> String
	{
		let statusStringMap: [UInt32: String] = [
			NoInfo.rawValue: 			"Download",
			DownloadQueued.rawValue: 	"Download Queued",
			Downloading.rawValue: 		"Downloading",
			DownloadError.rawValue: 	"Download Error",
			Extracting.rawValue: 		"Extracting",
			ExtractingError.rawValue: 	"Extract Error",
			Downloaded.rawValue: 		"Install",
			InstallQueued.rawValue:		"Install Queued",
			Installing.rawValue:		"Installing",
			InstallingError.rawValue:	"Install Error",
			Installed.rawValue:			"Installed"
		]
		
		return statusStringMap[vrpApp.Status.rawValue] ?? "Unknown"
	}
	
	private func DetermineButtonDisabled(vrpApp: SVrpApp) -> Bool
	{
		if (vrpApp.Status == NoInfo || vrpApp.Status == DownloadError || vrpApp.Status == ExtractingError){
			return false
		}
		
		if (selectedAdbDevice?.DeviceStatus == OK){
			if (vrpApp.Status == Downloaded || vrpApp.Status == InstallingError){
				return false
			}
		}
		
		return true
	}
	
	private func OnActionButtonClicked(vrpApp: SVrpApp)
	{
		// Download action
		if (vrpApp.Status == NoInfo || vrpApp.Status == DownloadError || vrpApp.Status == ExtractingError){
			onButtonDownloadAction?(vrpApp)
		}
		
		if (selectedAdbDevice?.DeviceStatus == OK){
			if (vrpApp.Status == Downloaded || vrpApp.Status == InstallingError){
				onButtonInstallAction?(vrpApp)
			}
		}
	}
	
}

struct VrpAppsGridView_Previews: PreviewProvider {
	@State static var vrpApps: [SVrpApp] = [
		SVrpApp(GameName: "Sample App With Loooonnoooooooooooooooooonnnnnnnnnnng Name. A very long name. So long it even needs 4 rows", ReleaseName: "Sample App Release Name", PackageName: "package.name", VersionCode: 123456, LastUpdated: "UTC 2024-12-31", SizeMB: 54551, Downloads: 0.5, Rating: 0.6, RatingCount: 137, Status: NoInfo, AppStatusParam: -1, StatusStr: "", Note: "This is a note", cAppPtr: nil, previewImage: NSImage(named: "ui_thumb_preview"))
	]
	
	@State static var selectedAdbDevice : SAdbDevice? = nil
	
    static var previews: some View {
		VrpAppsGridView(vrpApps: vrpApps, selectedAdbDevice: $selectedAdbDevice)
    }
}
