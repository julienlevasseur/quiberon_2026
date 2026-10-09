cmake_minimum_required(VERSION 3.16)
project(quiberon_bike_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../firmware/quiberon_bike/src)

# The portable core: everything in src/core builds on the PC as on the Uno.
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS ${SRC}/core/*.cpp)
add_library(core STATIC ${CORE_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp)
target_include_directories(core PUBLIC ${SRC})
# Core code must stay warning-free and C++11 so it also builds with avr-gcc.
set_target_properties(core PROPERTIES CXX_STANDARD 11 CXX_EXTENSIONS OFF)
target_compile_options(core PRIVATE -Wall -Wextra -Werror)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)
add_executable(tests main.cpp ${TEST_SOURCES})
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(tests PRIVATE -Wall -Wextra)
target_link_libraries(tests core)

enable_testing()
add_test(NAME unit COMMAND tests)

# Visual simulator (tools/sim): HTML replays of whole games on the simulated bench.
add_executable(sim ${CMAKE_CURRENT_SOURCE_DIR}/../tools/sim/sim.cpp)
target_include_directories(sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(sim PRIVATE -Wall -Wextra)
target_link_libraries(sim core)
