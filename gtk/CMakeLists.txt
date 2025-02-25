cmake_minimum_required(VERSION 3.10)

project(gtk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

# Compile xml resources
set(xml_resource_compile_script ../script/update_gtk_resources.sh)
add_custom_target(run_xml_resource_compile_script ALL
	COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/data ${xml_resource_compile_script}
	COMMENT "Compiling XML Resources to .c")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${xml_resource_compile_script})

add_executable(mloader-gtk	src/main.cpp
							src/SplashWindow.cpp
							src/MainWindow.cpp
							src/GtkGeneric.cpp
							src/resource/mloader_gtk_resources.c
)
target_link_libraries(mloader-gtk PRIVATE 	mloader
											PkgConfig::GTK3)

install(TARGETS mloader-gtk
		RUNTIME DESTINATION /usr/bin)

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

