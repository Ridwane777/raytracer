cmake_minimum_required(VERSION 3.28)
project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_COMPILE_FLAGS -Wall -Wextra -Werror -fno-gnu-unique)
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(raytracer ${SOURCES})

target_include_directories(raytracer PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(raytracer PRIVATE ${PROJECT_COMPILE_FLAGS})
target_link_libraries(raytracer PRIVATE dl)
set_target_properties(raytracer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
