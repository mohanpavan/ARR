add_executable(server server.cpp main.cpp)

target_link_libraries(server PRIVATE common)
install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)