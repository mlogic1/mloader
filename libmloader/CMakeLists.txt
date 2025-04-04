cmake_minimum_required(VERSION 3.10)

# Set the project name
project(libmloader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL REQUIRED)

set(MLOADER_VERSION_MAJOR 1)
set(MLOADER_VERSION_MINOR 0)
set(MLOADER_VERSION_PATCH 0)

# Add the library
add_library(mloader SHARED  src/AppContext.cpp
							src/SAContext.cpp
							src/VRPManager.cpp
							src/RClone.cpp
							src/7z.cpp
							src/ADB.cpp
							src/curl_global.cpp
							src/AdbDevice.cpp
							src/Logger.cpp
							src/QueueManager.cpp
							src/model/GameInfo.cpp
)

set(LIBMLOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(mloader PUBLIC ${LIBMLOADER_INCLUDE_DIR})
target_link_libraries(mloader PUBLIC CURL::libcurl)

target_compile_definitions(mloader PRIVATE	MLOADER_VERSION_MAJOR="${MLOADER_VERSION_MAJOR}"
											MLOADER_VERSION_MINOR="${MLOADER_VERSION_MINOR}"
											MLOADER_VERSION_PATCH="${MLOADER_VERSION_PATCH}")

install(TARGETS mloader
		LIBRARY DESTINATION /usr/lib)