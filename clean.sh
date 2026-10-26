#!/bin/bash

# Remove the build directory if it exists
if [ -d "build" ]; then
    rm -rf build
    echo "Removed 'build' directory."
fi

# Remove files generated at the root (in case cmake was run in-source)
rm -rf CMakeFiles
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f Makefile
rm -f compile_commands.json
rm -f *.ppm

# Remove compiled plugins
if [ -d "plugins" ]; then
    rm -f plugins/*.so
    echo "Removed plugins (.so)."
fi

# Remove the executable generated at the root (see CMakeLists.txt)
if [ -f "raytracer" ]; then
    rm raytracer
    echo "Removed 'raytracer' executable."
fi

echo "CMake cleanup done."
