cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MLoader)

# Add subdirectories for library and tools 
add_subdirectory(libmloader)
add_subdirectory(cli)
add_subdirectory(gtk)
