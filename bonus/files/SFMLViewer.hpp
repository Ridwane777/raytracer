#pragma once

#include <SFML/Graphics.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>
#include <string>
#include <memory>
#include <condition_variable>
#include <queue>

#include "../scene/Scene.hpp"
#include "../renderer/Renderer.hpp"
#include "../math/Vector3D.hpp"

namespace RayTracer {

// ─────────────────────────────────────────────────────────────
//  Tile : une bande de pixels à rendre (pour le thread pool)
// ─────────────────────────────────────────────────────────────
struct Tile {
    int startY;
    int endY;   // exclu
};

// ─────────────────────────────────────────────────────────────
//  ThreadPool : distribue les tiles aux workers
// ─────────────────────────────────────────────────────────────
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Vide l'ancienne file, remplit avec les nouvelles tâches et les lance.
    void submitTiles(
        std::vector<Tile> tiles,
        std::function<void(const Tile&, size_t)> task
    );

    // Attend que tous les workers aient fini.
    void waitAll();

    size_t threadCount() const { return _workers.size(); }

private:
    struct Job {
        Tile                                      tile;
        std::function<void(const Tile&, size_t)> task;
    };

    std::vector<std::thread>        _workers;
    std::queue<Job>                 _jobs;
    std::mutex                      _mutex;
    std::condition_variable         _cv;
    std::condition_variable         _doneCv;
    std::atomic<int>                _pending{0};
    bool                            _stop{false};

    void workerLoop(size_t id);
};

// ─────────────────────────────────────────────────────────────
//  RenderBuffer : double-buffer thread-safe
// ─────────────────────────────────────────────────────────────
class RenderBuffer {
public:
    RenderBuffer(int width, int height);

    // Écrit un pixel dans le buffer back.
    void setPixel(int x, int y, sf::Color color);

    // Swap back → front (appelé depuis le thread principal).
    void swap();

    // Copie le front buffer dans une sf::Texture.
    void uploadToTexture(sf::Texture& texture);

    int width()  const { return _width; }
    int height() const { return _height; }

private:
    int _width, _height;

    // Deux buffers de pixels bruts RGBA
    std::vector<sf::Uint8> _front;
    std::vector<sf::Uint8> _back;
    std::mutex             _swapMutex;
};

// ─────────────────────────────────────────────────────────────
//  CameraController : gestion des entrées utilisateur
// ─────────────────────────────────────────────────────────────
struct CameraController {
    // Vitesses
    float moveSpeed   = 5.0f;    // unités / seconde
    float rotSpeed    = 60.0f;   // degrés / seconde
    float scrollSpeed = 8.0f;    // unités par cran de molette

    // Applique les inputs clavier/molette à la caméra de la scène.
    // dt : delta temps en secondes.
    // Returns true si la caméra a bougé (rendu à relancer).
    bool update(Scene& scene, float dt,
                bool fwd, bool bwd, bool left, bool right,
                bool up, bool down,
                bool rotLeft, bool rotRight,
                bool rotUp, bool rotDown,
                float scrollDelta);
};

// ─────────────────────────────────────────────────────────────
//  SFMLViewer : fenêtre interactive
// ─────────────────────────────────────────────────────────────
class SFMLViewer {
public:
    struct Config {
        int         windowWidth  = 1280;
        int         windowHeight = 720;
        int         renderWidth  = 640;   // résolution de rendu (peut différer)
        int         renderHeight = 360;
        int         numThreads   = 0;     // 0 = auto (hardware_concurrency)
        int         maxDepth     = 0;
        std::string windowTitle  = "RayTracer — SFML Viewer";
        bool        showHUD      = true;
        bool        showTileGrid = false; // debug : affiche les tuiles
    };

    SFMLViewer(Scene scene, Config cfg = {});
    ~SFMLViewer() = default;

    // Bloque jusqu'à fermeture de la fenêtre.
    void run();

private:
    // ── État ──────────────────────────────────────────────────
    Scene          _scene;
    Config         _cfg;

    std::unique_ptr<ThreadPool>   _pool;
    std::unique_ptr<RenderBuffer> _buffer;
    sf::RenderWindow              _window;
    sf::Texture                   _texture;
    sf::Sprite                    _sprite;
    sf::Font                      _font;

    // Flags de rendu
    std::atomic<bool> _renderInProgress{false};
    std::atomic<bool> _cancelRender{false};
    std::atomic<int>  _tilesRemaining{0};
    std::atomic<int>  _totalTiles{0};

    // Contrôles
    CameraController _camCtrl;
    bool _keys[sf::Keyboard::KeyCount] = {};
    float _scrollAccum = 0.0f;

    // Statistiques
    double _lastRenderMs = 0.0;
    int    _renderCount  = 0;

    // ── Méthodes ──────────────────────────────────────────────
    void launchRender();
    void cancelAndWait();
    std::vector<Tile> buildTiles(int tileHeight = 32) const;

    void handleEvents();
    void handleInput(float dt);
    void drawHUD();
    void drawTileGrid();

    // Convertit Color (double [0,1]) → sf::Color
    static sf::Color toSFML(const Color& c);
};

} // namespace RayTracer
