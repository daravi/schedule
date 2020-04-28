cmake_minimum_required(VERSION 3.15)

project(scheduler
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_COMPILER ~/.local/bin/clang++)

add_library(scheduler SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/Scheduler.cpp)
target_include_directories(scheduler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_scheduler ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(test_scheduler PRIVATE scheduler)