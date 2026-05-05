#!/bin/bash

# Supprime le dossier build s'il existe
if [ -d "build" ]; then
    rm -rf build
    echo "Dossier 'build' supprimé."
fi

# Supprime les fichiers générés à la racine (au cas où vous avez lancé cmake .)
rm -rf CMakeFiles
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f Makefile
rm -f compile_commands.json

# Supprime les plugins compilés
if [ -d "plugins" ]; then
    rm -f plugins/*.so
    echo "Plugins (.so) supprimés."
fi

# Supprime l'exécutable généré à la racine (d'après votre CMakeLists.txt)
if [ -f "raytracer" ]; then
    rm raytracer
    echo "Exécutable 'raytracer' supprimé."
fi

echo "Nettoyage CMake terminé."
