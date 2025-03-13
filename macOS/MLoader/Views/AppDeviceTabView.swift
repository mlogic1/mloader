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
import AppKit

struct AppDeviceTabView: View {
	@Binding var previewImage: NSImage?
	@Binding var noteStr: String
	@Binding var deviceDetails: [SAdbDeviceDetails]
	
    var body: some View {
		TabView(){
			AppDetailsTabView(previewImage: $previewImage, noteStr: $noteStr)
				.tabItem(){
					Text("App details")
				}
			DeviceDetailsTabView(deviceDetails: $deviceDetails)
				.tabItem(){
					Text("Device details")
				}
		}
		.frame(height: 220)
    }
}

struct AppDetailsTabView: View {
	@Binding var previewImage: NSImage?
	@Binding var noteStr: String
	
	private var showNote: Bool{
		return !noteStr.isEmpty
	}
	
	var body: some View{
		HStack{
			VStack{
				Text("**Preview image**")
				Image(nsImage: previewImage!)
					.resizable()
					.aspectRatio(contentMode: .fit)
					.frame(width: 262, height: 150)
			}
			Spacer()
				.frame(maxWidth: 45.0)
			VStack{
				Text("**Note**")
				if showNote {
					Text(noteStr)
						.frame(height: 150)
				}else{
					Image("ui_note")
						.resizable()
						.aspectRatio(contentMode: .fit)
						.frame(width: 262, height: 150)
				}
			}
		}
	}
}

struct DeviceDetailsTabView: View {
	@Binding var deviceDetails: [SAdbDeviceDetails]
	
	var body: some View{
		HStack{
			VStack{
				Text("**Manufacturer**")
				Text(getProperty(for: "ro.product.manufacturer"))
				Spacer()
					.frame(maxHeight: 20.0)
				Text("**Model**")
				Text(getProperty(for: "ro.product.model"))
				Spacer()
					.frame(maxHeight: 20.0)
				Text("**Serial Number**")
				Text(getProperty(for: "ro.serialno"))
			}
			Spacer()
				.frame(maxWidth: 150.0)
			VStack{
				Text("**Android Version**")
				Text(getProperty(for: "ro.build.version.release"))
				Spacer()
					.frame(maxHeight: 20.0)
				Text("**SDK Version**")
				Text(getProperty(for: "ro.build.version.sdk"))
				Spacer()
					.frame(maxHeight: 20.0)
				Text("**OS Build Details**")
				Text(getProperty(for: "ro.build.branch"))
			}
		}
	}
	
	private func getProperty(for propertyName: String) -> String {
		if let detail = deviceDetails.first(where: { $0.PropertyName == propertyName }){
			if detail.PropertyValue.isEmpty{
				return "N/A"
			}else{
				return detail.PropertyValue
			}
		}
		return "N/A"
	}
}

struct AppDeviceTabView_Previews: PreviewProvider {
	@State static var staticPreview: NSImage? = NSImage(named: "ui_thumb_preview") ?? NSImage()
	@State static var notePreview: String = ""
	@State static var deviceDetails: [SAdbDeviceDetails] = []
	
    static var previews: some View {
		AppDeviceTabView(previewImage: $staticPreview, noteStr: $notePreview, deviceDetails: $deviceDetails)
    }
}
