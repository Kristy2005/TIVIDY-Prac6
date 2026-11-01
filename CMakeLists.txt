cmake_minimum_required(VERSION 3.10)
project(TIVIDY CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra)

# The reusable engine and industry scenarios do not depend on ncurses.
# The terminal application is built when requested and Curses is available.
option(TIVIDY_BUILD_TUI "Build the ncurses terminal user interface when available" ON)

# Everything in src/ EXCEPT main.cpp goes into the engine library,
# so tests can link against it without running main().
file(GLOB_RECURSE ENGINE_SRC src/*.cpp)
list(REMOVE_ITEM ENGINE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(tividy_engine ${ENGINE_SRC})
target_include_directories(tividy_engine PUBLIC src src/engine src/industries)

if(TIVIDY_BUILD_TUI)
    find_package(Curses QUIET)
    if(CURSES_FOUND)
        add_executable(tividy src/main.cpp)
        target_include_directories(tividy PRIVATE ${CURSES_INCLUDE_DIRS} ${CURSES_INCLUDE_DIR})
        target_link_libraries(tividy PRIVATE tividy_engine ${CURSES_LIBRARIES})
        message(STATUS "TIVIDY terminal interface enabled (Curses found)")
    else()
        message(WARNING
            "Curses was not found; skipping the TIVIDY terminal interface. "
            "The engine and tests will still be built. Install libncurses-dev "
            "or configure with -DTIVIDY_BUILD_TUI=OFF to silence this check.")
    endif()
else()
    message(STATUS "TIVIDY terminal interface disabled by TIVIDY_BUILD_TUI=OFF")
endif()

# Unit tests are independent of the TUI and Curses.
enable_testing()
file(GLOB_RECURSE TEST_SRC tests/*.cpp)
add_executable(tividy_tests ${TEST_SRC})
target_include_directories(tividy_tests PRIVATE tests)
target_link_libraries(tividy_tests tividy_engine)
add_test(NAME all_tests COMMAND tividy_tests)
