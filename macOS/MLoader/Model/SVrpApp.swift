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
import AppKit

// S(wift)VrpApp
struct SVrpApp : Identifiable, Equatable{
	let id = UUID()
	var GameName: String
	var ReleaseName: String
	var PackageName: String
	var VersionCode: Int32
	var LastUpdated: String
	var SizeMB: Int32
	var Downloads: Float
	var Rating: Float
	var RatingCount: Int32
	var Status: AppStatus
	var AppStatusParam: Int32
	var StatusStr: String
	var Note: String

	// Additional reference to the C struct
	var cAppPtr: UnsafeMutablePointer<VrpApp>?
	var previewImage: NSImage?
}
