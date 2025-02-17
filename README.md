# MLoader

An alternative sideloader tool for installing apps to Android based VR devices (Quest 2/Quest 3 etc..).
This sideloader is designed to run on Linux and MacOS and uses ROOKIE's public mirror.

## Table of Contents

 - [Notes](#notes)
 - [Screenshots](#screenshots)
 - [Usage](#usage)
 - [Contributing](#contributing)
 - [License](#license)
 - [Disclaimer](#disclaimer)

## Notes

This application is still early in development. There may be bugs or issues while using it.

### Development status
| OS      | Status            | Note   |
| ------- | ----------------- | ------ |
| Linux   | Early development |        |
| MacOS   | Not yet started   |        |

## Screenshots

## Usage

## Contributing
There are 3 tools/projects in this repository:
- **libmloader** - Core functionality of mloader built as a shared library. Its written in C++, although its public API (libmloader/include/mloader/) is written in C to support being used by high level languages like Python, C# or any others.
- **gtk** - Linux frontend application built with GTK
- **cli** - Command line version of MLoader
- **TODO** - Update this with mac version 

Currently there's a TODO file with briefly written todo items, but i'll move those to the Issues section. 

Most of the tools use CMake as their build system. 

Before building, make sure you have the the following dependencies installed (depending on which OS you want):

| Dependency | Linux | MacOS |
| --- | --- | --- |
| `curl` | Required | Required |
| `gtk-3.0` | Required| Not required |



## License
MLoader is distributed under GNU General Public License v3.0


## Disclaimer

This software is provided "as is", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and noninfringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software. 

By using this software, you agree that you understand these terms.
