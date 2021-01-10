cmake_minimum_required(VERSION 3.10)

project(GoServer VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(modules/spdlog)




include_directories("includes")

add_executable(GoServer src/main.cpp src/server.cpp src/game.cpp src/NetworkOperations.cpp src/board.cpp)

target_link_libraries(GoServer PUBLIC  spdlog::spdlog)
target_link_libraries(GoServer PUBLIC  spdlog::spdlog)

add_compile_options(-Wall )
