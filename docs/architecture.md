# Architecture - Raytracer

## Goal

The program generates a realistic image from a scene description file. It simulates the reverse
path of light: for each pixel of the final image, a ray is cast from the camera into the 3D scene.
If the ray hits an object, the pixel color is computed from the lights in the scene. The result is
written to a PPM file.

```
./raytracer scene.cfg   →   image.ppm
```

The core features are: Sphere, Plane, Translation, Directional light, Ambient light, Flat color,
PPM output. Intermediate features (Cylinder, Cone, Rotation, Drop shadows) and advanced ones
(Phong, Reflection, Refraction, Multithreading...) must stay reachable without redesigning the
architecture.

---

## Key architectural decisions

Four traps to avoid from the start so the intermediate/advanced features are not blocked:

### 1. Transformations through a Decorator, not through IPrimitive

Do not put `translate()` / `rotate()` / `scale()` in `IPrimitive`. Instead, a `Transform` class
wraps any primitive and applies a matrix to it. Adding rotation or scale never touches the
concrete primitives.

The idea: transform the incoming ray into the object's local space instead of moving the object
itself.

### 2. ILight receives the scene for drop shadows

`computeIntensity(point, normal)` is not enough for shadows: the light must be able to cast a
secondary ray towards itself to check whether an object lies between the hit point and the
source. `ILight::computeContribution` therefore receives the whole scene and a full `HitInfo`
(including the view direction, needed for Phong).

### 3. Recursive Renderer from the start

The render loop calls `computeColor(ray, depth)` with a depth parameter. At `depth 0` it is a flat
render. At `depth > 0` it handles reflections and refractions. No rewrite needed later.

### 4. Stateless Renderer for multithreading

The Renderer has no mutable state. `render()` returns a local buffer, never a class member.
Threads can work on independent pixel bands without a mutex.

---

## Layer overview

```
┌──────────────────────────────────────────┐
│               main.cpp                   │
└─────────────────┬────────────────────────┘
                  │
┌─────────────────▼────────────────────────┐
│            SceneParser                   │
│          + SceneBuilder  (Builder)       │
└─────────────────┬────────────────────────┘
                  │
┌─────────────────▼────────────────────────┐
│                Scene                     │
│  Camera  |  IPrimitive*  |  ILight*      │
│          (PrimitiveList) (LightList)     │
└─────────────────┬────────────────────────┘
                  │
┌─────────────────▼────────────────────────┐
│              Renderer                    │  stateless, recursive
└─────────────────┬────────────────────────┘
                  │
┌─────────────────▼────────────────────────┐
│             PPMWriter                    │
└──────────────────────────────────────────┘

Cross-cutting foundations:
  math/      →  Point3D, Vector3D, Ray, Rectangle3D, Color, Matrix4x4
  plugins/   →  .so loaded via dlopen (Sphere, Plane, DirectionalLight…)
```

---

## Math layer

### `Point3D`

```cpp
class Point3D {
public:
    double x, y, z;

    Point3D(double x = 0, double y = 0, double z = 0);

    Point3D  operator+(const Vector3D& v) const;
    Point3D  operator-(const Vector3D& v) const;
    Vector3D operator-(const Point3D& other) const;
    bool     operator==(const Point3D& other) const;
};
```

### `Vector3D`

```cpp
class Vector3D {
public:
    double x, y, z;

    Vector3D(double x = 0, double y = 0, double z = 0);

    Vector3D operator+(const Vector3D& other) const;
    Vector3D operator-(const Vector3D& other) const;
    Vector3D operator*(double scalar) const;
    Vector3D operator/(double scalar) const;
    bool     operator==(const Vector3D& other) const;

    double   dot(const Vector3D& other) const;
    Vector3D cross(const Vector3D& other) const;
    double   length() const;
    Vector3D normalize() const;
};
```

### `Ray`

```cpp
class Ray {
public:
    Point3D  origin;
    Vector3D direction; // always normalized

    Ray(const Point3D& origin, const Vector3D& direction);

    Point3D at(double t) const; // returns O + t*D
};
```

### `Rectangle3D`

Virtual image plane in the 3D scene. Used by the camera to convert pixels into 3D points.

```cpp
class Rectangle3D {
public:
    Point3D  origin;      // bottom-left corner
    Vector3D bottom_side; // vector to the bottom-right corner
    Vector3D left_side;   // vector to the top-left corner

    Rectangle3D(const Point3D& origin,
                const Vector3D& bottom_side,
                const Vector3D& left_side);

    // P = origin + u * bottom_side + v * left_side   (u, v in [0, 1])
    Point3D pointAt(double u, double v) const;
};
```

### `Color`

```cpp
class Color {
public:
    double r, g, b; // values in [0.0, 1.0]

    Color(double r = 0, double g = 0, double b = 0);

    Color operator+(const Color& other) const;
    Color operator*(double factor) const;
    Color operator*(const Color& other) const; // component-wise
    Color clamp() const;                       // forces into [0, 1]

    int toInt_r() const; // returns 0-255 for PPM output
    int toInt_g() const;
    int toInt_b() const;
};
```

### `Matrix4x4`

Needed from the start for the Transform Decorator. Handles translation, rotation and scale in a
single matrix. Allows composed transformations without modifying the primitives.

```cpp
class Matrix4x4 {
public:
    double m[4][4];

    Matrix4x4();                          // identity matrix
    static Matrix4x4 identity();
    static Matrix4x4 translation(const Vector3D& t);
    static Matrix4x4 rotationX(double deg);
    static Matrix4x4 rotationY(double deg);
    static Matrix4x4 rotationZ(double deg);
    static Matrix4x4 scale(double sx, double sy, double sz);

    Matrix4x4 operator*(const Matrix4x4& other) const;
    Matrix4x4 inverse() const;

    Point3D  transformPoint(const Point3D& p) const;
    Vector3D transformVector(const Vector3D& v) const;
    // Transforms a ray into the object's local space.
    Ray      transformRay(const Ray& ray) const;
};
```

---

## Interfaces layer

### `IPrimitive`

```cpp
class IPrimitive {
public:
    virtual ~IPrimitive() = default;

    // Returns nullopt if no intersection, otherwise t > 0 (distance along the ray).
    virtual std::optional<double> hits(const Ray& ray) const = 0;

    // Surface normal at the given point (normalized, pointing outward).
    virtual Vector3D getNormal(const Point3D& point) const = 0;

    // Flat color of the primitive at the given point.
    // The point is provided for procedural textures.
    virtual Color getColor(const Point3D& point) const = 0;

    // Advanced optical parameters (for Refraction / Reflection)
    virtual double getOpacity() const = 0;
    virtual double getRefractiveIndex() const = 0;
    virtual double getReflectivity() const = 0;
};
// No translate() here: transformations go through Transform (Decorator).
```

### `ILight` and `HitInfo`

```cpp
// Information about the hit point, passed to each light.
struct HitInfo {
    Point3D  point;     // intersection point in the scene
    Vector3D normal;    // surface normal at this point
    Vector3D viewDir;   // normalized direction towards the camera (for Phong)
};

class ILight {
public:
    virtual ~ILight() = default;

    // Computes this light's colored contribution at the hit point.
    // Receives the whole scene so it can cast shadow rays (drop shadows).
    // Returns Color(0,0,0) if the point is in shadow.
    virtual Color computeContribution(const HitInfo& hit,
                                      const IPrimitive& scene) const = 0;
};
```

---

## Design Pattern: Decorator - `Transform`

Wraps any `IPrimitive` and applies a transformation matrix to it. Adding rotation or scale
requires no change to the concrete primitives.

```cpp
class Transform : public IPrimitive {
private:
    IPrimitive* _wrapped;
    Matrix4x4   _worldToLocal; // inverse of the transformation matrix
    Matrix4x4   _localToWorld;

public:
    Transform(IPrimitive* primitive, const Matrix4x4& transform);

    // Transforms the ray into local space, delegates to _wrapped,
    // then maps the hit point back into world space.
    std::optional<double> hits(const Ray& ray) const override;

    // Transforms the normal from local to world space
    // (uses the inverse transpose to preserve orthogonality).
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;

    // Forwards optical parameters to _wrapped
    double getOpacity() const override;
    double getRefractiveIndex() const override;
    double getReflectivity() const override;
};
```

Usage in the parser:

```cpp
// A sphere with translation + rotation
IPrimitive* sphere = factory.create("sphere", params);
Matrix4x4 mat = Matrix4x4::translation({10, 0, 0}) * Matrix4x4::rotationY(45);
IPrimitive* transformed = new Transform(sphere, mat);
```

---

## Design Pattern: Builder - `SceneBuilder`

```cpp
class SceneBuilder {
private:
    std::optional<Camera>    _camera;
    std::vector<IPrimitive*> _primitives;
    std::vector<ILight*>     _lights;

public:
    SceneBuilder& setCamera(const Camera& camera);

    // Accepts a primitive that may already be wrapped in a Transform.
    SceneBuilder& addPrimitive(IPrimitive* primitive);
    SceneBuilder& addLight(ILight* light);

    // Checks that the camera is set, throws otherwise.
    // Wraps all primitives in a PrimitiveList (Composite).
    Scene build() const;
};
```

---

## Design Pattern: Factory - `PrimitiveFactory` / `LightFactory`

Loads `.so` plugins via `dlopen` and caches the handles.

```cpp
class PrimitiveFactory {
public:
    // Loads ./plugins/raytracer_{type}.so if not cached yet.
    // Calls createPrimitive(params) from the .so.
    IPrimitive* create(const std::string& type,
                       const std::map<std::string, double>& params);

    ~PrimitiveFactory(); // calls dlclose on all handles

private:
    std::map<std::string, void*> _handles;
    void* loadPlugin(const std::string& type);
};

class LightFactory {
public:
    ILight* create(const std::string& type,
                   const std::map<std::string, double>& params);
    ~LightFactory();

private:
    std::map<std::string, void*> _handles;
};
```

---

## Design Pattern: Composite - `PrimitiveList`

The Renderer calls `hits()` on a single object and gets the closest intersection among all
objects in the scene. Also essential for drop shadows: `ILight` receives the whole scene as an
`IPrimitive&` (which is actually this `PrimitiveList`).

```cpp
class PrimitiveList : public IPrimitive {
private:
    std::vector<IPrimitive*> _children;

public:
    void add(IPrimitive* primitive);

    // Returns the smallest t among all children (closest object).
    std::optional<double> hits(const Ray& ray) const override;

    // Returns the normal of the closest child at the given point.
    Vector3D getNormal(const Point3D& point) const override;

    // Returns the color of the closest child.
    Color getColor(const Point3D& point) const override;

    // Default implementation to satisfy the pure interface
    double getOpacity() const override;
    double getRefractiveIndex() const override;
    double getReflectivity() const override;

    // Access to the hit child (useful to fetch color + normal in the Renderer).
    const IPrimitive* getClosest(const Ray& ray) const;

    ~PrimitiveList(); // frees the children
};
```

---

## Camera layer

```cpp
class Camera {
public:
    Point3D  position;
    Vector3D rotation; // angles in degrees (x, y, z)
    int      width;
    int      height;
    double   fov;      // in degrees

    // 1. Default constructor
    Camera();

    // 2. Copy constructor
    Camera(const Camera& other);

    // 3. Assignment operator
    Camera& operator=(const Camera& other);

    // 4. Destructor
    ~Camera();
    Camera(const Point3D& position, const Vector3D& rotation,
           int width, int height, double fov);

    // Builds the Rectangle3D of the image plane.
    // Places the plane at distance 1 in front of the camera.
    // bottom_side and left_side are computed from the FOV and aspect ratio.
    Rectangle3D getScreen() const;

    // Generates the ray for pixel (px, py).
    // u = px / (width-1), v = py / (height-1)
    // direction = screen.pointAt(u, v) - position
    Ray getRayForPixel(int px, int py) const;
};
```

---

## Scene layer

```cpp
class Scene {
public:
    Camera        camera;
    PrimitiveList primitives; // Composite: acts as a single IPrimitive
    std::vector<ILight*> lights;

    Scene(const Camera& camera);

    void addPrimitive(IPrimitive* primitive); // delegates to primitives.add()
    void addLight(ILight* light);

    ~Scene();
};
```

---

## Concrete primitives (plugins)

Each plugin must export:

```cpp
extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void        destroyPrimitive(IPrimitive* p);
```

### `Sphere` - `plugins/raytracer_sphere.so`

```cpp
class Sphere : public IPrimitive {
private:
    Point3D _center;
    double  _radius;
    Color   _color;

public:
    Sphere(const Point3D& center, double radius, const Color& color);

    // Solves ||O + t*D - C||² = r²  →  at² + bt + c = 0
    // Returns the smallest positive t, or nullopt.
    std::optional<double> hits(const Ray& ray) const override;

    // (point - center).normalize()
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};
```

### `Plane` - `plugins/raytracer_plane.so`

```cpp
class Plane : public IPrimitive {
private:
    char   _axis;     // 'X', 'Y' or 'Z'
    double _position;
    Color  _color;

public:
    Plane(char axis, double position, const Color& color);

    // t = (position - origin[axis]) / direction[axis]
    // nullopt if direction[axis] ≈ 0 (ray parallel to the plane).
    std::optional<double> hits(const Ray& ray) const override;

    // Unit axis vector: (1,0,0) for X, (0,1,0) for Y, (0,0,1) for Z.
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};
```

### `Cylinder` - `plugins/raytracer_cylinder.so`

Added later without modifying any existing class.

```cpp
class Cylinder : public IPrimitive {
private:
    Point3D  _center;
    double   _radius;
    double   _height; // for the limited cylinder
    Color    _color;

public:
    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color    getColor(const Point3D& point) const override;
};
```

---

## Concrete lights (plugins)

```cpp
extern "C" ILight* createLight(const std::map<std::string, double>& params);
extern "C" void    destroyLight(ILight* l);
```

### `AmbientLight` - `plugins/raytracer_ambient.so`

```cpp
class AmbientLight : public ILight {
private:
    double _intensity;
    Color  _color;

public:
    // Always returns intensity * color, never shadowed.
    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;
};
```

### `DirectionalLight` - `plugins/raytracer_directional.so`

```cpp
class DirectionalLight : public ILight {
private:
    Vector3D _direction; // normalized, direction the light comes from
    double   _intensity;
    Color    _color;

public:
    // 1. dot = max(0, normal · (-direction))
    // 2. Drop shadows: cast a ray from point towards -direction,
    //    if scene.hits(shadowRay) returns a result → return Color(0,0,0)
    // 3. Return color * intensity * dot
    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;
};
```

---

## Renderer

Stateless and recursive from the start.

```cpp
class Renderer {
private:
    const Scene& _scene;
    int          _maxDepth; // 0 = flat render, 5 = reflections/refractions enabled

public:
    Renderer(const Scene& scene, int maxDepth = 0);

    // Runs the full render. Returns a 2D color buffer.
    // Stateless: can be parallelized by pixel bands (multithreading).
    std::vector<std::vector<Color>> render() const;

private:
    // Computes the color for a given ray.
    // depth >= _maxDepth → returns the background color (black).
    Color computeColor(const Ray& ray, int depth = 0) const;

    // Sums the contribution of all lights at the hit point.
    Color computeDirectLighting(const IPrimitive& primitive,
                                const HitInfo& info) const;

    // Reflections: builds the reflected ray and calls computeColor(depth+1).
    Color computeReflection(const HitInfo& info, int depth) const;

    // Refraction: builds the refracted ray and calls computeColor(depth+1).
    Color computeRefraction(const HitInfo& info, int depth) const;
};
```

---

## SceneParser

```cpp
class SceneParser {
private:
    PrimitiveFactory _primitiveFactory;
    LightFactory     _lightFactory;

public:
    // Reads the .cfg and builds the scene via SceneBuilder.
    Scene parse(const std::string& filepath);

private:
    Camera      parseCamera(const libconfig::Setting& cfg);
    IPrimitive* parsePrimitive(const libconfig::Setting& cfg);
    ILight*     parseLight(const libconfig::Setting& cfg);

    // Builds the transformation matrix from the translate/rotate/scale
    // fields of the .cfg, then wraps the primitive in a Transform.
    IPrimitive* applyTransforms(IPrimitive* primitive,
                                const libconfig::Setting& cfg);
};
```

---

## PPMWriter

```cpp
class PPMWriter {
public:
    // PPM P3 format (ASCII).
    // Header: "P3\nwidth height\n255\n"
    // Then: "r g b " per pixel, row by row.
    static void write(const std::string& filepath,
                      const std::vector<std::vector<Color>>& pixels);
};
```

---

## Class relationships

```
main
 └─► SceneParser ──► SceneBuilder ──────────────────► Scene
       │  applyTransforms()                             │
       │      └─► Transform (Decorator)                 ├── Camera
       ├─► PrimitiveFactory ──dlopen──► Sphere.so       ├── PrimitiveList (Composite)
       │                            └─► Plane.so        │     ├── Transform → Sphere*
       └─► LightFactory    ──dlopen──► Ambient.so       │     └── Transform → Plane*
                                    └─► Direct.so       └── ILight[]
                                                              ├── AmbientLight*
                                                              └── DirectionalLight*
Scene
 └─► Renderer
       ├── computeColor(ray, depth)
       │     ├── PrimitiveList.hits()            (Composite)
       │     ├── computeDirectLighting()
       │     │     └── ILight.computeContribution(hit, scene)
       │     │           └── shadowRay → PrimitiveList.hits()  (drop shadows)
       │     ├── computeReflection()   [depth > 0]
       │     └── computeRefraction()  [depth > 0]
       └─► PPMWriter.write()
```

---

## Extensibility

| Feature | What to do | What does not change |
|---|---|---|
| Cylinder, Cone | New `.so` plugin | IPrimitive, Renderer, Scene |
| Rotation | Transform + Matrix4x4 already exist | IPrimitive, concrete primitives |
| Drop shadows | DirectionalLight casts shadowRay via `scene` | ILight signature already planned |
| Phong | Uses `viewDir` already in HitInfo | ILight signature already planned |
| Reflection / Refraction | `computeReflection/Refraction()` in Renderer, `IPrimitive` extended with `getOpacity`, `getRefractiveIndex`, `getReflectivity` | Overall architecture (via the Transform Decorator) |
| Multithreading | Stateless Renderer → parallelize `render()` by bands | Everything else |
| Textures | `getColor(point)` already receives the point | IPrimitive signature already planned |
| Scale | Matrix4x4 already supports scale | Transform already exists |
| Multiple point lights | New `PointLight.so` plugin | ILight, Renderer |

---

## File layout

```
raytracer/
├── src/
│   ├── main.cpp
│   ├── math/
│   │   ├── Point3D.hpp / .cpp
│   │   ├── Vector3D.hpp / .cpp
│   │   ├── Ray.hpp / .cpp
│   │   ├── Rectangle3D.hpp / .cpp
│   │   ├── Color.hpp / .cpp
│   │   └── Matrix4x4.hpp / .cpp
│   ├── interfaces/
│   │   ├── IPrimitive.hpp
│   │   ├── ILight.hpp
│   │   └── HitInfo.hpp
│   ├── scene/
│   │   ├── Camera.hpp / .cpp
│   │   ├── Scene.hpp / .cpp
│   │   ├── SceneBuilder.hpp / .cpp      ← Builder
│   │   ├── PrimitiveList.hpp / .cpp     ← Composite
│   │   └── Transform.hpp / .cpp         ← Decorator
│   ├── factory/
│   │   ├── PrimitiveFactory.hpp / .cpp  ← Factory
│   │   └── LightFactory.hpp / .cpp
│   ├── parser/
│   │   └── SceneParser.hpp / .cpp
│   ├── renderer/
│   │   └── Renderer.hpp / .cpp
│   └── output/
│       └── PPMWriter.hpp / .cpp
└── plugins/
    ├── sphere/        Sphere.cpp           →  raytracer_sphere.so
    ├── plane/         Plane.cpp            →  raytracer_plane.so
    ├── cylinder/      Cylinder.cpp         →  raytracer_cylinder.so
    ├── cone/          Cone.cpp             →  raytracer_cone.so
    ├── ambient/       AmbientLight.cpp     →  raytracer_ambient.so
    └── directional/   DirectionalLight.cpp →  raytracer_directional.so
```

---

## Recommended implementation order

1. `Point3D`, `Vector3D`, `Color` - no dependencies
2. `Ray`, `Rectangle3D`, `Matrix4x4` - depend on Vector3D/Point3D
3. `IPrimitive`, `ILight`, `HitInfo` - the contracts to set first
4. `Sphere` + `Plane` (plugins) - first testable intersections
5. `Camera` + `Rectangle3D::pointAt` - ray generation
6. `PrimitiveList` (Composite) + `Transform` (Decorator)
7. `AmbientLight` + `DirectionalLight` (plugins)
8. `SceneBuilder` (Builder) + `PrimitiveFactory` + `LightFactory`
9. `SceneParser` (libconfig++)
10. `Renderer` (depth=0 first) + `PPMWriter`
11. Minimal test: one sphere, one ambient light, PPM output
12. Drop shadows: enable shadowRay in `DirectionalLight::computeContribution`
13. Cylinder, Cone: new plugins, nothing else to touch
14. Reflection/Refraction: pass `_maxDepth > 0` to the Renderer
