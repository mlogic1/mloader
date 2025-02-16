cmake_minimum_required(VERSION 3.10)

# Set the project name
project(libmloader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(CURL REQUIRED)

# Add the library
add_library(mloader SHARED  src/AppContext.cpp
                            src/VRPManager.cpp
                            src/RClone.cpp
                            src/7z.cpp
							src/ADB.cpp
                            src/curl_global.cpp
							src/AdbDevice.cpp
                            src/model/GameInfo.cpp
)

set(LIBMLOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(mloader PUBLIC ${LIBMLOADER_INCLUDE_DIR})
target_link_libraries(mloader PUBLIC CURL::libcurl)
