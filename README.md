# MLoader

A sideloader tool for Android based VR devices (Quest 2, Quest 3 etc...).  
This sideloader is designed to run on Linux and MacOS and also supports ROOKIE's public mirror.

## Table of Contents

 - [Notes](#notes)
 - [Screenshots](#screenshots)
 - [Building](#building)
 - [Contributing](#contributing)
 - [License](#license)
 - [Disclaimer](#disclaimer)

## Notes

This application is still early in development. There may be bugs or issues while using it.

### Development status
| OS      | Status            | Note               |
| ------- | ----------------- | ------------------ |
| Linux   | Early development |                    |
| MacOS   | Early development | No standalone mode |

## Screenshots
### (Kubuntu/KDE) Linux
Standalone Mode | VRP Mode
:-:|:-:
![](https://raw.githubusercontent.com/mlogic1/mloader/refs/heads/main/screenshots/screenshot_linux_standalone.png)  |  ![](https://raw.githubusercontent.com/mlogic1/mloader/refs/heads/main/screenshots/screenshot_linux_vrp.png)

### MacOS
![](https://raw.githubusercontent.com/mlogic1/mloader/refs/heads/main/screenshots/screenshot_mac_vrp.png)

## Building
There are 3 tools/projects in this repository:
- **libmloader** - Core functionality of mloader built as a shared library. Its written in C++, although its public API (libmloader/include/mloader/) is written in C to support being used by high level languages like Python, C# or any other.
- **gtk** - Linux frontend application built with GTK
- **macOS** - MacOS frontend application built with Swift/SwiftUI
- **cli** - Command line version of MLoader (mostly used for development and testing). By default it's not being built.

Most of the tools use CMake as their build system.

Before building, make sure you have the the following dependencies installed (depending on which OS you want):

| Dependency | Linux     | MacOS        |
| ---------- | --------- | ----------   |
| `cmake`    | Required  | Required     |
| `curl`     | Required  | Required     |
| `gtk-3.0`  | Required  | Not required |

### Linux
1. Check out the repository  
`git clone git@github.com:mlogic1/mloader.git`

2. move to the repository directory  
`cd mloader`

3. Make a build directory and cd into it  
`mkdir build && cd build`

4. Generate CMake build  
`cmake ..`

5. Build  
`make`

6. (Optional) Install to your system  
`sudo make install`


### MacOS
To be able to build on MacOS, libmloader cmake project must be generated in `build` directory of the repository
(This is where the Swift frontend project looks for libmloader)

1. Check out the repository  
`git clone git@github.com:mlogic1/mloader.git`

2. move to the repository directory  
`cd mloader`

3. Make a build directory and cd into it  
`mkdir build && cd build`

4. Generate XCode project  
`cmake -G Xcode ..`

5. Open the Swift project in macOS/MLoader.xcodeproj  

6. Build with Xcode


## Contributing
See [Issues](https://github.com/mlogic1/mloader/issues)

(Currently there's a TODO file with briefly written todo items, but i'll move those to the Issues section.)

## License
MLoader is distributed under GNU General Public License v3.0


## Disclaimer

This software is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and noninfringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.

By using this software, you agree that you understand these terms.
