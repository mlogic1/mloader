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

@main
struct MLoaderApp: App {
	@NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
	
    var body: some Scene {
		WindowGroup{
			MainWindow()
				.onAppear{
					NSWindow.allowsAutomaticWindowTabbing = false
					if let window = NSApplication.shared.windows.first {
						window.center()
					}
				}
		}
		.commands {
			CommandGroup(replacing: .newItem, addition: {
				Button("Preferences") {}
				Button("Delete all downloads") {}
			})
			CommandGroup(replacing: .undoRedo) { }
			CommandGroup(replacing: .pasteboard) { }
			CommandGroup(replacing: .saveItem) { }
			CommandGroup(replacing: .sidebar) { }
			CommandGroup(replacing: .textEditing) { }
		}
	}
}

class AppDelegate: NSObject, NSApplicationDelegate {
	func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
		// Return true so the app terminates after the last window is closed
		return true
	}
}
