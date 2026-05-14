#include "SFMLViewer.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace RayTracer {

// ═════════════════════════════════════════════════════════════
//  ThreadPool
// ═════════════════════════════════════════════════════════════

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i)
        _workers.emplace_back(&ThreadPool::workerLoop, this, i);
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _stop = true;
    }
    _cv.notify_all();
    for (auto& w : _workers)
        if (w.joinable()) w.join();
}

void ThreadPool::workerLoop(size_t id) {
    while (true) {
        Job job;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return _stop || !_jobs.empty(); });
            if (_stop && _jobs.empty()) return;
            job = std::move(_jobs.front());
            _jobs.pop();
        }
        job.task(job.tile, id);
        if (--_pending == 0)
            _doneCv.notify_all();
    }
}

void ThreadPool::submitTiles(
    std::vector<Tile> tiles,
    std::function<void(const Tile&, size_t)> task)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        // Vider l'ancienne file
        while (!_jobs.empty()) _jobs.pop();
        _pending = (int)tiles.size();
        for (auto& t : tiles)
            _jobs.push({t, task});
    }
    _cv.notify_all();
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(_mutex);
    _doneCv.wait(lock, [this] { return _pending == 0; });
}

// ═════════════════════════════════════════════════════════════
//  RenderBuffer
// ═════════════════════════════════════════════════════════════

RenderBuffer::RenderBuffer(int w, int h)
    : _width(w), _height(h),
      _front(w * h * 4, 0),
      _back (w * h * 4, 0)
{}

void RenderBuffer::setPixel(int x, int y, sf::Color color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    size_t idx = (y * _width + x) * 4;
    _back[idx + 0] = color.r;
    _back[idx + 1] = color.g;
    _back[idx + 2] = color.b;
    _back[idx + 3] = 255;
}

void RenderBuffer::swap() {
    std::lock_guard<std::mutex> lock(_swapMutex);
    std::swap(_front, _back);
}

void RenderBuffer::uploadToTexture(sf::Texture& texture) {
    std::lock_guard<std::mutex> lock(_swapMutex);
    texture.update(_front.data());
}

// ═════════════════════════════════════════════════════════════
//  CameraController
// ═════════════════════════════════════════════════════════════

bool CameraController::update(
    Scene& scene, float dt,
    bool fwd, bool bwd, bool left, bool right,
    bool up, bool down,
    bool rotLeft, bool rotRight,
    bool rotUp, bool rotDown,
    float scrollDelta)
{
    bool moved = false;

    Camera& cam = scene.camera;

    // ── Calcul des axes de la caméra ────────────────────────
    // La rotation est stockée en degrés (yaw = Y, pitch = X)
    double yaw   = cam.rotation.y * M_PI / 180.0;
    double pitch = cam.rotation.x * M_PI / 180.0;

    Vector3D forward{
        std::sin(yaw) * std::cos(pitch),
       -std::sin(pitch),
        std::cos(yaw) * std::cos(pitch)
    };
    Vector3D worldUp{0, 1, 0};
    Vector3D rightVec = forward.cross(worldUp).normalize();
    Vector3D upVec    = rightVec.cross(forward).normalize();

    // ── Translation ─────────────────────────────────────────
    float speed = moveSpeed * dt;

    auto move = [&](Vector3D dir) {
        cam.position = cam.position + dir;
        moved = true;
    };

    if (fwd)   move(forward  * speed);
    if (bwd)   move(forward  * (-speed));
    if (right) move(rightVec * speed);
    if (left)  move(rightVec * (-speed));
    if (up)    move(upVec    * speed);
    if (down)  move(upVec    * (-speed));

    // Scroll : avance / recule sur l'axe forward
    if (std::abs(scrollDelta) > 0.001f) {
        move(forward * (scrollDelta * scrollSpeed));
    }

    // ── Rotation ─────────────────────────────────────────────
    float rot = rotSpeed * dt;

    if (rotLeft)  { cam.rotation.y -= rot; moved = true; }
    if (rotRight) { cam.rotation.y += rot; moved = true; }
    if (rotUp)    { cam.rotation.x = std::max(-89.0, cam.rotation.x - rot); moved = true; }
    if (rotDown)  { cam.rotation.x = std::min( 89.0, cam.rotation.x + rot); moved = true; }

    return moved;
}

// ═════════════════════════════════════════════════════════════
//  SFMLViewer
// ═════════════════════════════════════════════════════════════

SFMLViewer::SFMLViewer(Scene scene, Config cfg)
    : _scene(std::move(scene)), _cfg(cfg)
{
    // Thread count
    int nThreads = cfg.numThreads > 0
        ? cfg.numThreads
        : (int)std::thread::hardware_concurrency();
    if (nThreads < 1) nThreads = 2;

    _pool   = std::make_unique<ThreadPool>(nThreads);
    _buffer = std::make_unique<RenderBuffer>(cfg.renderWidth, cfg.renderHeight);

    // Fenêtre SFML
    _window.create(
        sf::VideoMode(cfg.windowWidth, cfg.windowHeight),
        cfg.windowTitle,
        sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize
    );
    _window.setFramerateLimit(60);

    // Texture + Sprite (upscalé au besoin)
    _texture.create(cfg.renderWidth, cfg.renderHeight);
    _sprite.setTexture(_texture);
    float scaleX = (float)cfg.windowWidth  / cfg.renderWidth;
    float scaleY = (float)cfg.windowHeight / cfg.renderHeight;
    _sprite.setScale(scaleX, scaleY);

    // Police système
    if (!_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf") &&
        !_font.loadFromFile("/System/Library/Fonts/Menlo.ttc") &&
        !_font.loadFromFile("C:/Windows/Fonts/consola.ttf"))
    {
        std::cerr << "[SFMLViewer] Warning: could not load a monospace font. HUD disabled.\n";
        _cfg.showHUD = false;
    }

    std::cout << "[SFMLViewer] Using " << nThreads << " render threads.\n";
    std::cout << "[SFMLViewer] Render resolution: "
              << cfg.renderWidth << "x" << cfg.renderHeight << "\n";
    std::cout << "[SFMLViewer] Controls:\n"
              << "  ZQSD / Arrow keys  : move camera\n"
              << "  Mouse scroll       : move forward/backward\n"
              << "  Left/Right arrows  : rotate yaw\n"
              << "  R / F              : move up / down\n"
              << "  Page Up/Down       : rotate pitch\n"
              << "  +/-                : increase/decrease move speed\n"
              << "  H                  : toggle HUD\n"
              << "  G                  : toggle tile grid (debug)\n"
              << "  Escape             : quit\n";
}

// ─────────────────────────────────────────────────────────────

void SFMLViewer::run() {
    // Lancer le premier rendu
    launchRender();

    sf::Clock clock;

    while (_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.1f); // cap pour éviter les sauts après un rendu long

        handleEvents();
        handleInput(dt);

        // Upload du buffer (si prêt)
        _buffer->uploadToTexture(_texture);

        _window.clear(sf::Color(20, 20, 30));
        _window.draw(_sprite);

        if (_cfg.showHUD)      drawHUD();
        if (_cfg.showTileGrid) drawTileGrid();

        _window.display();
    }

    // Attendre la fin du rendu avant de détruire le pool
    cancelAndWait();
}

// ─────────────────────────────────────────────────────────────

std::vector<Tile> SFMLViewer::buildTiles(int tileHeight) const {
    std::vector<Tile> tiles;
    int h = _cfg.renderHeight;
    for (int y = 0; y < h; y += tileHeight) {
        tiles.push_back({y, std::min(y + tileHeight, h)});
    }
    return tiles;
}

void SFMLViewer::launchRender() {
    if (_renderInProgress.exchange(true)) {
        // Un rendu tourne déjà, on l'annule d'abord
        _cancelRender = true;
        _pool->waitAll();
        _cancelRender = false;
    }

    auto tiles = buildTiles(32);
    _totalTiles     = (int)tiles.size();
    _tilesRemaining = (int)tiles.size();

    // Capture des données nécessaires (snapshot de la scène)
    Scene       sceneCopy = _scene;        // copie légère
    int         rw        = _cfg.renderWidth;
    int         rh        = _cfg.renderHeight;
    int         maxDepth  = _cfg.maxDepth;
    RenderBuffer* buf     = _buffer.get();
    auto& cancel          = _cancelRender;
    auto& remaining       = _tilesRemaining;
    auto& inProgress      = _renderInProgress;

    auto start = std::chrono::steady_clock::now();
    auto* lastMs = &_lastRenderMs;

    _pool->submitTiles(std::move(tiles),
        [sceneCopy, rw, rh, maxDepth, buf, &cancel, &remaining, &inProgress, start, lastMs]
        (const Tile& tile, size_t /*threadId*/) mutable
    {
        if (cancel.load()) {
            --remaining;
            return;
        }

        // Renderer stateless (comme décrit dans l'architecture)
        Renderer renderer(sceneCopy, maxDepth);

        for (int y = tile.startY; y < tile.endY; ++y) {
            if (cancel.load()) break;
            for (int x = 0; x < rw; ++x) {
                Ray   ray   = sceneCopy.camera.getRayForPixel(x, y);
                Color color = renderer.computeColorPublic(ray, 0);
                buf->setPixel(x, y, SFMLViewer::toSFML(color));
            }
        }

        // Swap progressif : chaque tile mise à jour immédiatement
        buf->swap();

        if (--remaining == 0) {
            auto end = std::chrono::steady_clock::now();
            *lastMs  = std::chrono::duration<double, std::milli>(end - start).count();
            inProgress = false;
        }
    });
}

void SFMLViewer::cancelAndWait() {
    _cancelRender = true;
    _pool->waitAll();
    _cancelRender = false;
    _renderInProgress = false;
}

// ─────────────────────────────────────────────────────────────

void SFMLViewer::handleEvents() {
    sf::Event event;
    while (_window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                cancelAndWait();
                _window.close();
                break;

            case sf::Event::KeyPressed:
                _keys[event.key.code] = true;

                if (event.key.code == sf::Keyboard::Escape) {
                    cancelAndWait();
                    _window.close();
                }
                if (event.key.code == sf::Keyboard::H)
                    _cfg.showHUD = !_cfg.showHUD;
                if (event.key.code == sf::Keyboard::G)
                    _cfg.showTileGrid = !_cfg.showTileGrid;

                // Vitesse
                if (event.key.code == sf::Keyboard::Add ||
                    event.key.code == sf::Keyboard::Equal)
                    _camCtrl.moveSpeed *= 1.5f;
                if (event.key.code == sf::Keyboard::Subtract ||
                    event.key.code == sf::Keyboard::Hyphen)
                    _camCtrl.moveSpeed = std::max(0.1f, _camCtrl.moveSpeed / 1.5f);
                break;

            case sf::Event::KeyReleased:
                _keys[event.key.code] = false;
                break;

            case sf::Event::MouseWheelScrolled:
                _scrollAccum += event.mouseWheelScroll.delta;
                break;

            case sf::Event::Resized: {
                sf::FloatRect visibleArea(0, 0,
                    (float)event.size.width, (float)event.size.height);
                _window.setView(sf::View(visibleArea));
                float sx = (float)event.size.width  / _cfg.renderWidth;
                float sy = (float)event.size.height / _cfg.renderHeight;
                _sprite.setScale(sx, sy);
                break;
            }
            default: break;
        }
    }
}

void SFMLViewer::handleInput(float dt) {
    using K = sf::Keyboard;

    bool fwd      = _keys[K::Z] || _keys[K::W] || _keys[K::Up];
    bool bwd      = _keys[K::S] || _keys[K::Down];
    bool left     = _keys[K::Q] || _keys[K::A];
    bool right    = _keys[K::D];
    bool up       = _keys[K::R] || _keys[K::Space];
    bool down     = _keys[K::F] || _keys[K::LControl];
    bool rotLeft  = _keys[K::Left];
    bool rotRight = _keys[K::Right];
    bool rotUp    = _keys[K::PageUp];
    bool rotDown  = _keys[K::PageDown];

    float scroll    = _scrollAccum;
    _scrollAccum    = 0.0f;

    bool moved = _camCtrl.update(
        _scene, dt,
        fwd, bwd, left, right, up, down,
        rotLeft, rotRight, rotUp, rotDown,
        scroll
    );

    if (moved) {
        ++_renderCount;
        launchRender();
    }
}

// ─────────────────────────────────────────────────────────────

void SFMLViewer::drawHUD() {
    const Camera& cam = _scene.camera;
    int tiles    = _totalTiles.load();
    int rem      = _tilesRemaining.load();
    int done     = tiles - rem;
    float pct    = tiles > 0 ? (float)done / tiles * 100.f : 100.f;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "FPS  : " << (int)(1.0f / std::max(0.001f, (float)(_lastRenderMs/1000.0))) << "\n";
    oss << "Render: " << _lastRenderMs << " ms\n";
    oss << "Threads: " << _pool->threadCount() << "\n";
    oss << "Tiles: " << done << "/" << tiles << " (" << pct << "%)\n";
    oss << "Renders: " << _renderCount << "\n";
    oss << "---\n";
    oss << "Pos: ("
        << cam.position.x << ", "
        << cam.position.y << ", "
        << cam.position.z << ")\n";
    oss << "Rot: ("
        << cam.rotation.x << ", "
        << cam.rotation.y << ", "
        << cam.rotation.z << ")\n";
    oss << "Speed: " << _camCtrl.moveSpeed << "\n";
    oss << "---\n";
    oss << "ZQSD/arrows: move\n";
    oss << "Scroll: fwd/bwd\n";
    oss << "PageUp/Down: pitch\n";
    oss << "Left/Right: yaw\n";
    oss << "+/- : speed\n";
    oss << "H: toggle HUD\n";

    // Fond semi-transparent
    sf::RectangleShape bg;
    bg.setSize({280.f, 260.f});
    bg.setPosition(8.f, 8.f);
    bg.setFillColor(sf::Color(0, 0, 0, 160));
    bg.setOutlineColor(sf::Color(80, 200, 120, 180));
    bg.setOutlineThickness(1.f);
    _window.draw(bg);

    sf::Text text;
    text.setFont(_font);
    text.setString(oss.str());
    text.setCharacterSize(13);
    text.setFillColor(sf::Color(180, 255, 180));
    text.setPosition(14.f, 12.f);
    _window.draw(text);

    // Barre de progression
    if (_renderInProgress.load()) {
        float barW = 280.f;
        float barH = 6.f;
        sf::RectangleShape barBg({barW, barH});
        barBg.setPosition(8.f, 270.f);
        barBg.setFillColor(sf::Color(40, 40, 40));
        _window.draw(barBg);

        sf::RectangleShape barFg({barW * pct / 100.f, barH});
        barFg.setPosition(8.f, 270.f);
        barFg.setFillColor(sf::Color(80, 220, 100));
        _window.draw(barFg);
    }
}

void SFMLViewer::drawTileGrid() {
    // Affiche les bordures de chaque tile pour le debug
    float scaleX = (float)_window.getSize().x / _cfg.renderWidth;
    float scaleY = (float)_window.getSize().y / _cfg.renderHeight;

    auto tiles = buildTiles(32);
    for (const auto& tile : tiles) {
        float y0 = tile.startY * scaleY;
        float y1 = tile.endY   * scaleY;
        float x0 = 0.f;
        float x1 = (float)_window.getSize().x;

        sf::Vertex line[] = {
            {{x0, y0}, sf::Color(255, 100, 0, 60)},
            {{x1, y0}, sf::Color(255, 100, 0, 60)}
        };
        _window.draw(line, 2, sf::Lines);
        sf::Vertex line2[] = {
            {{x0, y1}, sf::Color(255, 100, 0, 60)},
            {{x1, y1}, sf::Color(255, 100, 0, 60)}
        };
        _window.draw(line2, 2, sf::Lines);
    }
}

// ─────────────────────────────────────────────────────────────

/*static*/ sf::Color SFMLViewer::toSFML(const Color& c) {
    Color clamped = c.clamp();
    return sf::Color(
        (sf::Uint8)(clamped.r * 255),
        (sf::Uint8)(clamped.g * 255),
        (sf::Uint8)(clamped.b * 255),
        255
    );
}

} // namespace RayTracer
