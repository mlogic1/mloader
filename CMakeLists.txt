cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MLoader)

# Basic OS detection
if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

# Add subdirectories for library and tools 
add_subdirectory(libmloader)
add_subdirectory(cli)

if (${LINUX})
	add_subdirectory(gtk)
endif()
