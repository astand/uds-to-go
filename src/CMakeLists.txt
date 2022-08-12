cmake_minimum_required(VERSION 3.5)

project(${project} LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(udsrun
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-receiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/docan-sender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/iso-tp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/isotp/pci-helper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uds/diag/diag-router.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/d-timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/etc/timers/tickerup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp
)

target_include_directories(
  udsrun PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/etc
)


# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG release-1.12.1
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# enable_testing()

# add_executable(
#   cctest
#   ${CMAKE_CURRENT_SOURCE_DIR}/helpers/formatter.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/options-parser.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/tests/args-test.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/tests/bitext-test.cpp
# )

# target_link_libraries(
#   cctest
#   GTest::gtest_main
# )

# include(GoogleTest)
# gtest_discover_tests(cctest)


# add_library(
#   dummy OBJECT 
#   ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/usr/testdb-fmon.c
#   ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/lib/testdb.c
#   ${CMAKE_CURRENT_SOURCE_DIR}/../test/gencode/butl/bms_testdb-binutil.c
# )

