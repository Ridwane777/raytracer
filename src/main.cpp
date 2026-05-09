#include "Error.hpp"
#include "parser/SceneParser.hpp"
#include "scene/SceneBuilder.hpp"
#include "renderer/Renderer.hpp"
#include "output/PPMWriter.hpp"

#include <iostream>
#include <string>

static void usage()
{
    std::cout << "USAGE: ./raytracer <SCENE_FILE>\n"
              << "   SCENE_FILE: scene configuration (.cfg)" << std::endl;
}

int main(int ac, char** av)
{
    try {
        if (ac != 2)
            throw Error("Invalid number of arguments.");

        std::string arg = av[1];

        if (arg == "--help") {
            usage();
            return 0;
        }

        if (arg.find(".cfg") == std::string::npos)
            throw Error("Unknown argument or file format (expected .cfg).");
        SceneParser parser;
        SceneConfig config = parser.parse(arg);
        SceneBuilder builder("plugins");
        Scene scene = builder.build(config);
        Renderer renderer(scene, 0);
        auto pixels = renderer.render();
        PPMWriter::write("output.ppm", pixels);

        std::cout << "Render complete → output.ppm ("
                  << scene.camera.width << "x" << scene.camera.height
                  << ")" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}