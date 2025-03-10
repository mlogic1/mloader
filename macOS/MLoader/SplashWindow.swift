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
import Foundation

@_silgen_name("InitializeMLoader")
func heheinitialize() -> CInt

// This can be deleted


// @_silgen_name("MLoaderGetLibraryVersion")
// func MLoaderGetLibraryVersion() -> String
// typealias verFunc = @convention(c) (CInt, CInt) -> CInt
typealias verFunc = @convention(c) () -> UnsafePointer<CChar>

struct SplashWindow: View {
	
	@State var initStatus : String = ""
	
    var body: some View {
		VStack {
			ZStack(alignment: .bottomTrailing){
				Image("splash_image")
				Text(initStatus)
					.foregroundColor(Color.black)
					.padding(.horizontal, 20)
			}
			Button("BTN", action: ButtonAction)
		}
		/*.onAppear{
			InitializeMLoader()
		}*/
	}
	
	func InitializeMLoader()
	{
		initStatus = "Initialized"
	}
	
	func ButtonAction()
	{
		// let hihi : String = MLoaderGetLibraryVersion()
		
		// let path : String = "/Users/filip/Projects/mloader/build/libmloader/Debug/libmloader.dylib"
		let path : String = "libmloader.dylib"
		if let handle = dlopen(path, RTLD_NOW) {
			if let sym = dlsym(handle, "MLoaderGetLibraryVersion") {
				let f = unsafeBitCast(sym, to: verFunc.self)
				// let result = f(12, 45)
				let resultPointer = f()
				
				if let result = String(validatingUTF8: resultPointer) {
					initStatus = result
					print(result)
				} else {
					print("Failed to convert C string to Swift string")
				}
			}
			dlclose(handle)
		}
		
		/*
		NSApp.windows.first?.close() // Close the current window
		var windowRef : NSWindow
		windowRef = NSWindow(
				contentRect: NSRect(x: 100, y: 100, width: 100, height: 600),
				styleMask: [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView],
				backing: .buffered, defer: false)
		windowRef.contentView = NSHostingView(rootView: MainWindow())
		windowRef.makeKeyAndOrderFront(nil)
		 */
	}
}

struct SplashWindow_Previews: PreviewProvider {
    static var previews: some View {
        SplashWindow()
    }
}
