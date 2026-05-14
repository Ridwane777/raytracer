# ─────────────────────────────────────────────────────────────
# Ajout SFML dans votre CMakeLists.txt existant
# ─────────────────────────────────────────────────────────────
# Coller dans CMakeLists.txt, après la définition de la cible principale :

# --- SFML Bonus (optionnel) ---
option(BUILD_SFML_VIEWER "Build the interactive SFML viewer" ON)

if(BUILD_SFML_VIEWER)
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

    if(SFML_FOUND)
        # Cible séparée pour le viewer interactif
        add_executable(raytracer_sfml
            src/main_sfml.cpp        # point d'entrée sfml (voir ci-dessous)
            src/bonus/SFMLViewer.cpp
            # + tous les .cpp du projet principal sauf main.cpp
            src/math/Point3D.cpp
            src/math/Vector3D.cpp
            src/math/Ray.cpp
            src/math/Rectangle3D.cpp
            src/math/Color.cpp
            src/math/Matrix4x4.cpp
            src/scene/Camera.cpp
            src/scene/Scene.cpp
            src/scene/SceneBuilder.cpp
            src/scene/PrimitiveList.cpp
            src/scene/Transform.cpp
            src/factory/PrimitiveFactory.cpp
            src/factory/LightFactory.cpp
            src/parser/SceneParser.cpp
            src/renderer/Renderer.cpp
            src/output/PPMWriter.cpp
        )
        target_include_directories(raytracer_sfml PRIVATE src)
        target_link_libraries(raytracer_sfml
            PRIVATE
            sfml-graphics sfml-window sfml-system
            ${CMAKE_DL_LIBS}      # dlopen
        )
        target_compile_features(raytracer_sfml PRIVATE cxx_std_17)
        message(STATUS "SFML viewer: enabled (SFML ${SFML_VERSION})")
    else()
        message(STATUS "SFML viewer: SFML not found, skipping (install libsfml-dev)")
    endif()
endif()

# ─────────────────────────────────────────────────────────────
# Makefile alternatif (si vous n'utilisez pas CMake)
# ─────────────────────────────────────────────────────────────
# Ajouter à votre Makefile existant :

SFML_FLAGS  = -lsfml-graphics -lsfml-window -lsfml-system
SFML_SRC    = src/bonus/SFMLViewer.cpp src/main_sfml.cpp

raytracer_sfml: $(OBJ) $(SFML_SRC:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(SFML_FLAGS)

# Installation SFML :
#   Ubuntu/Debian : sudo apt install libsfml-dev
#   macOS         : brew install sfml
#   Windows       : vcpkg install sfml
