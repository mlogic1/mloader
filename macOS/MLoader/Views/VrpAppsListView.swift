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

struct VrpAppsListView: View {
	@Binding var selectedVrpApp: SVrpApp.ID?
	var vrpApps: [SVrpApp]
	
    var body: some View {
		Table(vrpApps, selection: $selectedVrpApp){
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
    }
}

struct VrpAppsListView_Previews: PreviewProvider {
	@State static var vrpApps: [SVrpApp] = []
	@State static var selectedVrpApp: SVrpApp.ID? = nil
    static var previews: some View {
        VrpAppsListView(selectedVrpApp: $selectedVrpApp, vrpApps: vrpApps)
    }
}
