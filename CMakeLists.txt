cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MLoader)

# Add subdirectories for library, encrypter and decrypter
add_subdirectory(libmloader)
add_subdirectory(cli)