cmake_minimum_required(VERSION 3.28)
project(raytracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_COMPILE_FLAGS -Wall -Wextra -Werror -fno-gnu-unique)

# --- Plugins ---
set(PLUGINS sphere plane cylinder cone ambient directional point)

foreach(PLUGIN ${PLUGINS})
    file(GLOB PLUGIN_SOURCES "plugins/${PLUGIN}/*.cpp")
    add_library(raytracer_${PLUGIN} SHARED ${PLUGIN_SOURCES})
    target_include_directories(raytracer_${PLUGIN} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_options(raytracer_${PLUGIN} PRIVATE ${PROJECT_COMPILE_FLAGS})
    set_target_properties(raytracer_${PLUGIN} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
        PREFIX ""
    )
endforeach()

# --- Raytracer principal ---
file(GLOB_RECURSE SOURCES "src/*.cpp")
# Exclure main_sfml.cpp du binaire principal (double main sinon)
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main_sfml.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/sfml/SFMLViewer.cpp")

add_executable(raytracer ${SOURCES})
target_include_directories(raytracer PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
target_compile_options(raytracer PRIVATE ${PROJECT_COMPILE_FLAGS})
target_link_libraries(raytracer PRIVATE dl config++)
target_link_options(raytracer PRIVATE -rdynamic)
set_target_properties(raytracer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# --- SFML Viewer (bonus) ---
option(BUILD_SFML_VIEWER "Build the interactive SFML viewer" ON)

if(BUILD_SFML_VIEWER)
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

    if(SFML_FOUND)
        file(GLOB_RECURSE SFML_CORE_SOURCES "src/*.cpp")
        list(REMOVE_ITEM SFML_CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

        add_executable(raytracer_sfml
            ${SFML_CORE_SOURCES}
            sfml/SFMLViewer.cpp
        )
        target_include_directories(raytracer_sfml PRIVATE
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/sfml
        )
        target_compile_options(raytracer_sfml PRIVATE ${PROJECT_COMPILE_FLAGS})
        target_link_libraries(raytracer_sfml
            PRIVATE
            sfml-graphics sfml-window sfml-system
            dl config++
        )
        target_link_options(raytracer_sfml PRIVATE -rdynamic)
        target_compile_features(raytracer_sfml PRIVATE cxx_std_20)
        set_target_properties(raytracer_sfml PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
        )
        message(STATUS "SFML viewer: enabled (SFML ${SFML_VERSION})")
    else()
        message(STATUS "SFML viewer: SFML not found, skipping (install libsfml-dev)")
    endif()
endif()
