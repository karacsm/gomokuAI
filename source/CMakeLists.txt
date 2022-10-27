cmake_minimum_required(VERSION 3.23.1)

project(Gomoku)

add_executable(Gomoku gomoku.cpp)

target_link_libraries(Gomoku d2d1)

message(STATUS ${CMAKE_INSTALL_PREFIX})

install(FILES "${CMAKE_SOURCE_DIR}/threat_table.txt" DESTINATION "${CMAKE_INSTALL_PREFIX}/Gomoku")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION "${CMAKE_INSTALL_PREFIX}/Gomoku")
install(TARGETS Gomoku DESTINATION "${CMAKE_INSTALL_PREFIX}/Gomoku")