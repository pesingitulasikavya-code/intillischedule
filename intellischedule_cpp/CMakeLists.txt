cmake_minimum_required(VERSION 3.20)
project(IntelliSchedule VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compiler Flags
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic -O2)
endif()

# Threading (required by cpp-httplib)
find_package(Threads REQUIRED)

# SQLite3 Library
find_package(SQLite3 REQUIRED)

# Common library sources
set(COMMON_SOURCES
    src/algorithms/knapsack_dp.cpp
    src/algorithms/knapsack_greedy.cpp
    src/algorithms/interval_scheduler.cpp
    src/algorithms/graph_scheduler.cpp
    src/validator.cpp
    src/database.cpp
)

# 1. CLI Executable (Used by Node/Express integration and command line pipelines)
add_executable(intellischedule_cli
    src/cli_engine.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(intellischedule_cli PRIVATE SQLite::SQLite3 Threads::Threads)

# 2. Standalone HTTP Server (Port 8080 standalone C++ microservice)
add_executable(intellischedule_server
    src/server_main.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(intellischedule_server PRIVATE SQLite::SQLite3 Threads::Threads)

# 3. Automated Test Suite Runner
add_executable(intellischedule_tests
    tests/test_runner.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(intellischedule_tests PRIVATE SQLite::SQLite3 Threads::Threads)

enable_testing()
add_test(NAME ComprehensiveVerificationSuite COMMAND intellischedule_tests)
