cmake_minimum_required(VERSION 3.10)
project(ARR)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE PATH "Default installation directory" FORCE)
endif()

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
