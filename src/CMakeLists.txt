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
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/session/session-control.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/session/uds-server-base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/d-timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/tickerup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/iso-tp-server/serv-factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/app-helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp
)

target_include_directories(
  udstogo-test PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/etc
)

add_executable(ecu-server-test
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-receiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-sender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-tp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/pci-helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/session/session-control.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/session/uds-server-base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/session/apps/did-router.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/d-timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/tickerup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/uds-test-server/serv-factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/uds-test-server/test-uds-server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/uds-test-server/session-client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/app-helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/server-app.cpp
)

target_include_directories(
  ecu-server-test PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/etc
)
  
find_package(Threads REQUIRED)
target_link_libraries(udstogo-test PRIVATE Threads::Threads)
target_link_libraries(ecu-server-test PRIVATE Threads::Threads)


# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  test_run
  ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/d-timer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/tickerup.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gtests/etc/timer-tests.cpp
)

target_link_libraries(
  test_run
  GTest::gtest_main
)

target_include_directories(
  test_run PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/
)

include(GoogleTest)
gtest_discover_tests(test_run)
