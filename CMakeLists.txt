cmake_minimum_required(VERSION 3.10)
project(TIVIDY CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra)

# Everything in src/ EXCEPT main.cpp goes into the engine library,
# so tests can link against it without running main().
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM ENGINE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(tividy_engine ${ENGINE_SRC})
target_include_directories(tividy_engine PUBLIC src src/engine)

# The application (TUI)
find_package(Curses REQUIRED)
add_executable(tividy src/main.cpp)
target_link_libraries(tividy PRIVATE tividy_engine)
target_include_directories(tividy PRIVATE ${CURSES_INCLUDE_DIR})
target_link_libraries(tividy PRIVATE ${CURSES_LIBRARIES})

# Unit tests
enable_testing()
file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS tests/*.cpp)
add_executable(tividy_tests ${TEST_SRC})
target_include_directories(tividy_tests PRIVATE tests)
target_link_libraries(tividy_tests tividy_engine)
add_test(NAME all_tests COMMAND tividy_tests)