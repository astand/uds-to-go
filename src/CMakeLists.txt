cmake_minimum_required(VERSION 3.5)

project(${project} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(udstogo-test
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-receiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-sender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-tp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/pci-helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/diag/diag-router.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/d-timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/tickerup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp
)

target_include_directories(
  udstogo-test PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/etc
)
  
find_package(Threads REQUIRED)
target_link_libraries(udstogo-test PRIVATE Threads::Threads)
