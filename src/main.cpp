#include "Error.hpp"
#include "parser/SceneParser.hpp"
#include "scene/SceneBuilder.hpp"
#include "renderer/Renderer.hpp"
#include "output/PPMWriter.hpp"
#ifdef RAYTRACER_HAS_SFML
#include "../sfml/SFMLViewer.hpp"
#endif

#include <iostream>
#include <string>

static void usage(const char* argv0)
{
    std::cout
        << "USAGE: " << argv0 << " <SCENE_FILE> [options]\n"
        << "   SCENE_FILE: scene configuration (.cfg)\n"
        << "Options:\n"
        << "  --sfml             Launch the interactive SFML viewer\n"
        << "  --threads N        Number of render threads (SFML mode, default: auto)\n"
        << "  --depth D          Ray recursion depth (default: 4)\n"
        << "  --res WxH          Override the scene resolution (e.g. 640x360)\n"
        << "  --no-hud           Disable the overlay HUD (SFML mode)\n"
        << "  --tile-debug       Show tile boundaries (SFML mode)\n"
        << "  -o, --output FILE  Output image path (default: output.ppm)\n"
        << "  --help / -h        Show this help message\n";
}

int main(int ac, char** av)
{
    try {
        if (ac < 2) {
            usage(av[0]);
            return 84;
        }

        std::string arg1 = av[1];
        if (arg1 == "--help" || arg1 == "-h") {
            usage(av[0]);
            return 0;
        }

        std::string scenePath;
        bool        sfmlMode   = false;
        int         numThreads = 0;
        int         maxDepth   = 4;
        int         renderW    = 0;
        int         renderH    = 0;
        bool        showHUD    = true;
        bool        tileDebug  = false;
        std::string outputPath = "output.ppm";

        for (int i = 1; i < ac; ++i) {
            std::string a = av[i];
            if (a == "--sfml") {
                sfmlMode = true;
            } else if ((a == "-o" || a == "--output") && i + 1 < ac) {
                outputPath = av[++i];
            } else if ((a == "--threads") && i + 1 < ac) {
                numThreads = std::stoi(av[++i]);
            } else if ((a == "--depth") && i + 1 < ac) {
                maxDepth = std::stoi(av[++i]);
            } else if ((a == "--res") && i + 1 < ac) {
                std::string res = av[++i];
                auto x = res.find('x');
                if (x == std::string::npos) x = res.find('X');
                if (x != std::string::npos) {
                    renderW = std::stoi(res.substr(0, x));
                    renderH = std::stoi(res.substr(x + 1));
                }
            } else if (a == "--no-hud") {
                showHUD = false;
            } else if (a == "--tile-debug") {
                tileDebug = true;
            } else if (a == "--help" || a == "-h") {
                usage(av[0]);
                return 0;
            } else if (a[0] != '-') {
                if (scenePath.empty())
                    scenePath = a;
                else
                    throw Error("Unexpected argument: " + a);
            } else {
                throw Error("Unknown option: " + a);
            }
        }

        if (scenePath.empty())
            throw Error("No scene file provided.");
        if (scenePath.find(".cfg") == std::string::npos)
            throw Error("Expected a .cfg scene file.");
        SceneParser  parser;
        SceneConfig  config = parser.parse(scenePath);
        SceneBuilder builder("plugins");
        Scene        scene  = builder.build(config);

#ifndef RAYTRACER_HAS_SFML
        (void)numThreads;
        (void)showHUD;
        (void)tileDebug;
        if (sfmlMode)
            throw Error("This build has no SFML viewer (SFML 2 was not found).");
#else
        if (sfmlMode) {
            RayTracer::SFMLViewer::Config cfg;
            cfg.renderWidth  = renderW > 0 ? renderW : config.camera.resolution.width;
            cfg.renderHeight = renderH > 0 ? renderH : config.camera.resolution.height;
            cfg.windowWidth  = std::min(cfg.renderWidth  * 2, 1920);
            cfg.windowHeight = std::min(cfg.renderHeight * 2, 1080);
            cfg.numThreads   = numThreads;
            cfg.maxDepth     = maxDepth;
            cfg.showHUD      = showHUD;
            cfg.showTileGrid = tileDebug;
            cfg.windowTitle  = "RayTracer - " + scenePath;

            std::cout << "[raytracer] SFML mode\n"
                      << "  Scene   : " << scenePath << "\n"
                      << "  Render  : " << cfg.renderWidth << "x" << cfg.renderHeight << "\n"
                      << "  Threads : " << (numThreads == 0 ? "auto" : std::to_string(numThreads)) << "\n"
                      << "  Depth   : " << maxDepth << "\n";

            RayTracer::SFMLViewer viewer(std::move(scene), cfg);
            viewer.run();
        } else
#endif
        {
            if (renderW > 0 && renderH > 0) {
                scene.camera.width  = renderW;
                scene.camera.height = renderH;
            }
            Renderer renderer(scene, maxDepth);
            auto     pixels = renderer.render();
            PPMWriter::write(outputPath, pixels);
            std::cout << "Render complete: " << outputPath << " ("
                      << scene.camera.width << "x" << scene.camera.height
                      << ")" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}