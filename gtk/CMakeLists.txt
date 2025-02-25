cmake_minimum_required(VERSION 3.10)

project(gtk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(mloader-gtk	src/main.cpp
							src/SplashWindow.cpp
							src/MainWindow.cpp
							src/GtkGeneric.cpp
)
target_link_libraries(mloader-gtk PRIVATE 	mloader
											PkgConfig::GTK3)


set(GTK_DATA_DIR ${CMAKE_BINARY_DIR}/gtk/)
file(MAKE_DIRECTORY ${GTK_DATA_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data
	DESTINATION ${GTK_DATA_DIR})

install(TARGETS mloader-gtk
		RUNTIME DESTINATION /usr/bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/layouts
		DESTINATION /usr/share/mloader-gtk)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/ui
		DESTINATION /usr/share/mloader-gtk)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/16x16/mloader.png
		DESTINATION /usr/share/icons/hicolor/16x16/apps/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/32x32/mloader.png
		DESTINATION /usr/share/icons/hicolor/32x32/apps/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/48x48/mloader.png
		DESTINATION /usr/share/icons/hicolor/48x48/apps/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/64x64/mloader.png
		DESTINATION /usr/share/icons/hicolor/64x64/apps/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/icons/256x256/mloader.png
		DESTINATION /usr/share/icons/hicolor/256x256/apps/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/mloader.desktop
		DESTINATION /usr/share/applications)

