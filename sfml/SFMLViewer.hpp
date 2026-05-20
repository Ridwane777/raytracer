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

#include "../src/scene/Scene.hpp"
#include "../src/renderer/Renderer.hpp"
#include "../src/math/Vector3D.hpp"

namespace RayTracer {


struct Tile {
    int startY;
    int endY;   // exclu
};

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    void submitTiles(
        std::vector<Tile> tiles,
        std::function<void(const Tile&, size_t)> task
    );

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


class RenderBuffer {
public:
    RenderBuffer(int width, int height);
    void setPixel(int x, int y, sf::Color color);
    void uploadToTexture(sf::Texture& texture); 
    int width()  const { return _width; }
    int height() const { return _height; }
private:
    int _width, _height;
    std::vector<sf::Uint8> _pixels;
    std::mutex             _mutex; 
};

struct CameraController {
    float moveSpeed   = 5.0f;
    float rotSpeed    = 60.0f;
    float scrollSpeed = 8.0f;

    bool update(Scene& scene, float dt,
                bool fwd, bool bwd, bool left, bool right,
                bool up, bool down,
                bool rotLeft, bool rotRight,
                bool rotUp, bool rotDown,
                float scrollDelta);
};


class SFMLViewer {
public:
    struct Config {
        int         windowWidth  = 1280;
        int         windowHeight = 720;
        int         renderWidth  = 640;
        int         renderHeight = 360;
        int         numThreads   = 0;
        int         maxDepth     = 0;
        std::string windowTitle  = "RayTracer — SFML Viewer";
        bool        showHUD      = true;
        bool        showTileGrid = false; 
    };

    SFMLViewer(Scene scene, Config cfg);
    ~SFMLViewer() = default;

    void run();

private:
    Scene          _scene;
    Config         _cfg;

    std::unique_ptr<ThreadPool>   _pool;
    std::unique_ptr<RenderBuffer> _buffer;
    sf::RenderWindow              _window;
    sf::Texture                   _texture;
    sf::Sprite                    _sprite;
    sf::Font                      _font;

    std::atomic<bool> _renderInProgress{false};
    std::atomic<bool> _cancelRender{false};
    std::atomic<int>  _tilesRemaining{0};
    std::atomic<int>  _totalTiles{0};

    CameraController _camCtrl;
    bool _keys[sf::Keyboard::KeyCount] = {};
    float _scrollAccum = 0.0f;

    double _lastRenderMs = 0.0;
    int    _renderCount  = 0;

    void launchRender();
    void cancelAndWait();
    std::vector<Tile> buildTiles(int tileHeight = 32) const;

    void handleEvents();
    void handleInput(float dt);
    void drawHUD();
    void drawTileGrid();

    static sf::Color toSFML(const Color& c);
};

}
