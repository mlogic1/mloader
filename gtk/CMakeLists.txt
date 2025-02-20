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