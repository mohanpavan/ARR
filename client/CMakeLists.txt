add_executable(client client.cpp main.cpp)

target_link_libraries(client PRIVATE common)
install(TARGETS client DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)