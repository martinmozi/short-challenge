cmake_minimum_required(VERSION 2.7)
project(Challenge)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(thread-synchronization thread-synchronization/main.cpp)
add_executable(dispatcher dispatcher/dispatcher.cpp)
target_link_libraries(thread-synchronization ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(dispatcher )
