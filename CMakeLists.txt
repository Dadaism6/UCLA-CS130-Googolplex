cmake_minimum_required(VERSION 3.10.0)
project(googolplex)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log regex filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs whenever new source files are added
add_library(session src/session.cc src/log.cc)
add_library(server_src src/server.cc src/session.cc src/log.cc)
add_library(config_parser src/config_parser.cc src/log.cc)
add_library(request_parser src/http/request_parser.cc)
add_library(reply src/http/reply.cc)
add_library(logger src/log.cc)

add_library(request_handler src/request_handler_echo.cc src/request_handler_static.cc src/log.cc)

# Update executable name, srcs, and deps, remember to add new 
# libraries to target_link_libraries
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver logger session server_src config_parser request_parser request_handler reply pthread Boost::system Boost::filesystem Boost::log_setup Boost::log)

# Update test executable name, srcs, and deps
# add_executable(example_lib_test tests/example_lib_test.cc)
# target_link_libraries(example_lib_test example_lib gtest_main)
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test logger config_parser gtest_main Boost::system Boost::filesystem Boost::log_setup Boost::log)

add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test logger server_src request_parser request_handler reply gtest_main gmock_main Boost::filesystem Boost::system Boost::log_setup Boost::log)

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test logger session request_parser request_handler reply gtest_main gmock_main Boost::filesystem Boost::system Boost::log_setup Boost::log)

add_executable(request_handler_test tests/request_handler_test.cc)
target_link_libraries(request_handler_test logger request_handler reply request_parser gtest_main Boost::filesystem Boost::log_setup Boost::log)

# Update with test binary
# gtest_discover_tests(example_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_test(NAME ServerIntegrationTest COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration.sh)
add_test(NAME ServerIntegrationTest2 COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_bad.sh)

# Update with target/test targets
# include(cmake/CodeCoverageReportConfig.cmake)
# generate_coverage_report(TARGETS example_server example_lib TESTS example_lib_test)
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS session server_src config_parser request_handler TESTS config_parser_test server_test session_test request_handler_test)
