// src/main_sfml.cpp
// Point d'entrée pour le viewer SFML interactif.
// Usage : ./raytracer_sfml scene.cfg [--threads N] [--depth D] [--res WxH]

#include <iostream>
#include <string>
#include <stdexcept>

#include "parser/SceneParser.hpp"
#include "bonus/SFMLViewer.hpp"

static void printUsage(const char* argv0) {
    std::cerr
        << "Usage: " << argv0 << " <scene.cfg> [options]\n"
        << "Options:\n"
        << "  --threads N      Number of render threads (default: auto)\n"
        << "  --depth D        Ray recursion depth (default: 0)\n"
        << "  --res WxH        Render resolution (default: camera size)\n"
        << "                   Example: --res 640x360\n"
        << "  --no-hud         Disable the overlay HUD\n"
        << "  --tile-debug     Show tile boundaries\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string scenePath = argv[1];

    // ── Valeurs par défaut ────────────────────────────────────
    int  numThreads  = 0;   // 0 = auto
    int  maxDepth    = 0;
    int  renderW     = 0;   // 0 = prendre la taille de la caméra
    int  renderH     = 0;
    bool showHUD     = true;
    bool tileDebug   = false;

    // ── Parsing des arguments ─────────────────────────────────
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--threads" && i + 1 < argc) {
            numThreads = std::stoi(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            maxDepth = std::stoi(argv[++i]);
        } else if (arg == "--res" && i + 1 < argc) {
            std::string res = argv[++i];
            auto x = res.find('x');
            if (x == std::string::npos) x = res.find('X');
            if (x != std::string::npos) {
                renderW = std::stoi(res.substr(0, x));
                renderH = std::stoi(res.substr(x + 1));
            }
        } else if (arg == "--no-hud") {
            showHUD = false;
        } else if (arg == "--tile-debug") {
            tileDebug = true;
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    // ── Chargement de la scène ────────────────────────────────
    RayTracer::Scene scene = [&] {
        try {
            RayTracer::SceneParser parser;
            return parser.parse(scenePath);
        } catch (const std::exception& e) {
            std::cerr << "Error loading scene: " << e.what() << "\n";
            std::exit(1);
        }
    }();

    // ── Config viewer ─────────────────────────────────────────
    RayTracer::SFMLViewer::Config cfg;

    // Si --res non spécifié, utiliser la résolution de la caméra
    cfg.renderWidth  = renderW > 0 ? renderW : scene.camera.width;
    cfg.renderHeight = renderH > 0 ? renderH : scene.camera.height;

    // Fenêtre : au minimum la résolution de rendu, au maximum 1920x1080
    cfg.windowWidth  = std::min(cfg.renderWidth  * 2, 1920);
    cfg.windowHeight = std::min(cfg.renderHeight * 2, 1080);

    cfg.numThreads   = numThreads;
    cfg.maxDepth     = maxDepth;
    cfg.showHUD      = showHUD;
    cfg.showTileGrid = tileDebug;
    cfg.windowTitle  = "RayTracer — " + scenePath;

    std::cout << "[main_sfml] Scene   : " << scenePath << "\n"
              << "[main_sfml] Render  : " << cfg.renderWidth
                                  << "x" << cfg.renderHeight << "\n"
              << "[main_sfml] Window  : " << cfg.windowWidth
                                  << "x" << cfg.windowHeight << "\n"
              << "[main_sfml] Threads : "
              << (numThreads == 0 ? "auto" : std::to_string(numThreads)) << "\n"
              << "[main_sfml] Depth   : " << maxDepth << "\n";

    // ── Lancement ─────────────────────────────────────────────
    RayTracer::SFMLViewer viewer(std::move(scene), cfg);
    viewer.run();

    return 0;
}
