# Architecture — Raytracer

## Objectif du projet

Le but est de créer un programme capable de générer une image réaliste à partir d'un fichier de
description de scène. Le programme simule le trajet inverse de la lumière : pour chaque pixel de
l'image finale, un rayon est envoyé depuis la caméra dans la scène 3D. Si ce rayon touche un
objet, la couleur du pixel est calculée en fonction des lumières présentes. Le résultat est écrit
dans un fichier PPM.

```
./raytracer scene.cfg   →   image.ppm
```

Les fonctionnalités obligatoires (`must`) sont : Sphere, Plane, Translation, Directional light,
Ambient light, Flat color, sortie PPM. Les fonctionnalités `should` (Cylinder, Cone, Rotation,
Drop shadows) et `could` (Phong, Reflection, Refraction, Multithreading...) doivent rester
atteignables sans refonte de l'architecture.

---

## Décisions architecturales clés

Quatre pièges à éviter dès le départ pour ne pas bloquer les `should`/`could` :

### 1. Transformations via Decorator, pas via IPrimitive

Ne pas mettre `translate()` / `rotate()` / `scale()` dans `IPrimitive`. À la place, une classe
`Transform` enveloppe n'importe quelle primitive et lui applique une matrice. Ajouter la rotation
(should) ou le scale (could) ne touche jamais aux primitives concrètes.

Le principe : on transforme le rayon entrant dans le repère local de l'objet, plutôt que de
déplacer l'objet lui-même.

### 2. ILight reçoit la scène pour les ombres portées

`computeIntensity(point, normal)` ne suffit pas pour les ombres (should) : la lumière doit pouvoir
envoyer un rayon secondaire vers elle-même pour vérifier si un objet est entre le point touché et
la source. `ILight::computeContribution` reçoit donc la scène entière et un `HitInfo` complet
(incluant la direction de la caméra, utile pour Phong).

### 3. Renderer récursif dès le départ

La boucle de rendu appelle `computeColor(ray, depth)` avec un paramètre de profondeur. À `depth 0`
c'est un rendu plat. À `depth > 0` c'est les reflets et réfractions (could). Aucune réécriture
nécessaire plus tard.

### 4. Renderer stateless pour le multithreading

Le Renderer n'a aucun état mutable. `render()` retourne un buffer local, jamais un attribut de
classe. Les threads peuvent travailler sur des bandes de pixels indépendantes sans mutex.

---

## Vue d'ensemble des couches

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
│              Renderer                    │  stateless, récursif
└─────────────────┬────────────────────────┘
                  │
┌─────────────────▼────────────────────────┐
│             PPMWriter                    │
└──────────────────────────────────────────┘

Fondations transversales :
  math/      →  Point3D, Vector3D, Ray, Rectangle3D, Color, Matrix4x4
  plugins/   →  .so chargés via dlopen (Sphere, Plane, DirectionalLight…)
```

---

## Couche Math

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
    Vector3D direction; // toujours normalisée

    Ray(const Point3D& origin, const Vector3D& direction);

    Point3D at(double t) const; // retourne O + t*D
};
```

### `Rectangle3D`

Plan image virtuel dans la scène 3D. Utilisé par la caméra pour convertir les pixels en points 3D.

```cpp
class Rectangle3D {
public:
    Point3D  origin;      // coin bas-gauche
    Vector3D bottom_side; // vecteur vers le coin bas-droit
    Vector3D left_side;   // vecteur vers le coin haut-gauche

    Rectangle3D(const Point3D& origin,
                const Vector3D& bottom_side,
                const Vector3D& left_side);

    // P = origin + u * bottom_side + v * left_side   (u, v dans [0, 1])
    Point3D pointAt(double u, double v) const;
};
```

### `Color`

```cpp
class Color {
public:
    double r, g, b; // valeurs dans [0.0, 1.0]

    Color(double r = 0, double g = 0, double b = 0);

    Color operator+(const Color& other) const;
    Color operator*(double factor) const;
    Color operator*(const Color& other) const; // composante par composante
    Color clamp() const;                       // force dans [0, 1]

    int toInt_r() const; // retourne 0-255 pour l'écriture PPM
    int toInt_g() const;
    int toInt_b() const;
};
```

### `Matrix4x4`

Nécessaire dès le départ pour le Decorator Transform. Gère translation, rotation et scale en une
seule matrice. Permet les transformations composées sans modifier les primitives.

```cpp
class Matrix4x4 {
public:
    double m[4][4];

    Matrix4x4();                          // matrice identité
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
    // Transforme un rayon dans le repère local de l'objet.
    Ray      transformRay(const Ray& ray) const;
};
```

---

## Couche Interfaces

### `IPrimitive`

```cpp
class IPrimitive {
public:
    virtual ~IPrimitive() = default;

    // Retourne nullopt si pas d'intersection, sinon t > 0 (distance le long du rayon).
    virtual std::optional<double> hits(const Ray& ray) const = 0;

    // Normale à la surface au point donné (normalisée, vers l'extérieur).
    virtual Vector3D getNormal(const Point3D& point) const = 0;

    // Couleur flat de la primitive au point donné.
    // Le point est fourni pour les textures procédurales (could).
    virtual Color getColor(const Point3D& point) const = 0;
};
// Pas de translate() ici : les transformations passent par Transform (Decorator).
```

### `ILight` et `HitInfo`

```cpp
// Informations sur le point touché, passées à chaque lumière.
struct HitInfo {
    Point3D  point;     // point d'intersection dans la scène
    Vector3D normal;    // normale de la surface en ce point
    Vector3D viewDir;   // direction normalisée vers la caméra (pour Phong)
};

class ILight {
public:
    virtual ~ILight() = default;

    // Calcule la contribution colorée de cette lumière au point touché.
    // Reçoit la scène entière pour pouvoir envoyer des rayons d'ombre (drop shadows).
    // Retourne Color(0,0,0) si le point est dans l'ombre.
    virtual Color computeContribution(const HitInfo& hit,
                                      const IPrimitive& scene) const = 0;
};
```

---

## Design Pattern : Decorator — `Transform`

Enveloppe n'importe quelle `IPrimitive` et lui applique une matrice de transformation. Ajouter la
rotation ou le scale n'impose aucune modification aux primitives concrètes.

```cpp
class Transform : public IPrimitive {
private:
    IPrimitive* _wrapped;
    Matrix4x4   _worldToLocal; // inverse de la matrice de transformation
    Matrix4x4   _localToWorld;

public:
    Transform(IPrimitive* primitive, const Matrix4x4& transform);

    // Transforme le rayon dans le repère local, délègue à _wrapped,
    // puis retransforme le point de retour dans le repère monde.
    std::optional<double> hits(const Ray& ray) const override;

    // Transforme la normale du repère local vers le repère monde
    // (utilise la transposée de l'inverse pour conserver l'orthogonalité).
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};
```

Usage dans le parser :

```cpp
// Une sphère avec translation + rotation
IPrimitive* sphere = factory.create("sphere", params);
Matrix4x4 mat = Matrix4x4::translation({10, 0, 0}) * Matrix4x4::rotationY(45);
IPrimitive* transformed = new Transform(sphere, mat);
```

---

## Design Pattern : Builder — `SceneBuilder`

```cpp
class SceneBuilder {
private:
    std::optional<Camera>    _camera;
    std::vector<IPrimitive*> _primitives;
    std::vector<ILight*>     _lights;

public:
    SceneBuilder& setCamera(const Camera& camera);

    // Accepte déjà une primitive potentiellement wrappée dans un Transform.
    SceneBuilder& addPrimitive(IPrimitive* primitive);
    SceneBuilder& addLight(ILight* light);

    // Vérifie que la caméra est définie, lève une exception sinon.
    // Enveloppe toutes les primitives dans un PrimitiveList (Composite).
    Scene build() const;
};
```

---

## Design Pattern : Factory — `PrimitiveFactory` / `LightFactory`

Charge les plugins `.so` via `dlopen`, met les handles en cache.

```cpp
class PrimitiveFactory {
public:
    // Charge ./plugins/raytracer_{type}.so si pas encore en cache.
    // Appelle createPrimitive(params) depuis le .so.
    IPrimitive* create(const std::string& type,
                       const std::map<std::string, double>& params);

    ~PrimitiveFactory(); // appelle dlclose sur tous les handles

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

## Design Pattern : Composite — `PrimitiveList`

Le Renderer appelle `hits()` sur un seul objet et obtient l'intersection la plus proche parmi tous
les objets de la scène. Essentiel aussi pour les ombres portées : `ILight` reçoit la scène entière
sous forme d'`IPrimitive&` (qui est en réalité ce `PrimitiveList`).

```cpp
class PrimitiveList : public IPrimitive {
private:
    std::vector<IPrimitive*> _children;

public:
    void add(IPrimitive* primitive);

    // Retourne le t le plus petit parmi tous les enfants (objet le plus proche).
    std::optional<double> hits(const Ray& ray) const override;

    // Retourne la normale de l'enfant le plus proche au point donné.
    Vector3D getNormal(const Point3D& point) const override;

    // Retourne la couleur de l'enfant le plus proche.
    Color getColor(const Point3D& point) const override;

    // Accès à l'enfant touché (utile pour récupérer couleur + normale dans le Renderer).
    const IPrimitive* getClosest(const Ray& ray) const;

    ~PrimitiveList(); // libère les enfants
};
```

---

## Couche Camera

```cpp
class Camera {
public:
    Point3D  position;
    Vector3D rotation; // angles en degrés (x, y, z)
    int      width;
    int      height;
    double   fov;      // en degrés

    Camera(const Point3D& position, const Vector3D& rotation,
           int width, int height, double fov);

    // Construit le Rectangle3D du plan image.
    // Place le plan à distance 1 devant la caméra.
    // bottom_side et left_side sont calculés depuis le FOV et le ratio.
    Rectangle3D getScreen() const;

    // Génère le rayon pour le pixel (px, py).
    // u = px / (width-1), v = py / (height-1)
    // direction = screen.pointAt(u, v) - position
    Ray getRayForPixel(int px, int py) const;
};
```

---

## Couche Scene

```cpp
class Scene {
public:
    Camera        camera;
    PrimitiveList primitives; // Composite : agit comme une seule IPrimitive
    std::vector<ILight*> lights;

    Scene(const Camera& camera);

    void addPrimitive(IPrimitive* primitive); // délègue à primitives.add()
    void addLight(ILight* light);

    ~Scene();
};
```

---

## Primitives concrètes (plugins)

Chaque plugin exporte obligatoirement :

```cpp
extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params);
extern "C" void        destroyPrimitive(IPrimitive* p);
```

### `Sphere` — `plugins/raytracer_sphere.so`

```cpp
class Sphere : public IPrimitive {
private:
    Point3D _center;
    double  _radius;
    Color   _color;

public:
    Sphere(const Point3D& center, double radius, const Color& color);

    // Résout ||O + t*D - C||² = r²  →  at² + bt + c = 0
    // Retourne le t positif le plus petit, ou nullopt.
    std::optional<double> hits(const Ray& ray) const override;

    // (point - center).normalize()
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};
```

### `Plane` — `plugins/raytracer_plane.so`

```cpp
class Plane : public IPrimitive {
private:
    char   _axis;     // 'X', 'Y' ou 'Z'
    double _position;
    Color  _color;

public:
    Plane(char axis, double position, const Color& color);

    // t = (position - origin[axis]) / direction[axis]
    // nullopt si direction[axis] ≈ 0 (rayon parallèle au plan).
    std::optional<double> hits(const Ray& ray) const override;

    // Vecteur unitaire de l'axe : (1,0,0) pour X, (0,1,0) pour Y, (0,0,1) pour Z.
    Vector3D getNormal(const Point3D& point) const override;

    Color getColor(const Point3D& point) const override;
};
```

### `Cylinder` (should) — `plugins/raytracer_cylinder.so`

Ajouté plus tard sans modifier aucune classe existante.

```cpp
class Cylinder : public IPrimitive {
private:
    Point3D  _center;
    double   _radius;
    double   _height; // pour le cylindre limité (could)
    Color    _color;

public:
    std::optional<double> hits(const Ray& ray) const override;
    Vector3D getNormal(const Point3D& point) const override;
    Color    getColor(const Point3D& point) const override;
};
```

---

## Lumières concrètes (plugins)

```cpp
extern "C" ILight* createLight(const std::map<std::string, double>& params);
extern "C" void    destroyLight(ILight* l);
```

### `AmbientLight` — `plugins/raytracer_ambient.so`

```cpp
class AmbientLight : public ILight {
private:
    double _intensity;
    Color  _color;

public:
    // Retourne toujours intensity * color, jamais d'ombre.
    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;
};
```

### `DirectionalLight` — `plugins/raytracer_directional.so`

```cpp
class DirectionalLight : public ILight {
private:
    Vector3D _direction; // normalisée, direction d'où vient la lumière
    double   _intensity;
    Color    _color;

public:
    // 1. dot = max(0, normal · (-direction))
    // 2. Drop shadows (should) : envoie un rayon depuis point vers -direction,
    //    si scene.hits(shadowRay) a un résultat → retourne Color(0,0,0)
    // 3. Retourne color * intensity * dot
    Color computeContribution(const HitInfo& hit,
                              const IPrimitive& scene) const override;
};
```

---

## Renderer

Stateless et récursif dès le départ.

```cpp
class Renderer {
private:
    const Scene& _scene;
    int          _maxDepth; // 0 = rendu plat, 5 = reflets/réfractions actifs

public:
    Renderer(const Scene& scene, int maxDepth = 0);

    // Lance le rendu complet. Retourne un buffer 2D de couleurs.
    // Stateless : peut être parallélisé par bandes de pixels (multithreading).
    std::vector<std::vector<Color>> render() const;

private:
    // Calcule la couleur pour un rayon donné.
    // depth >= _maxDepth → retourne la couleur de fond (noir).
    Color computeColor(const Ray& ray, int depth = 0) const;

    // Additionne la contribution de toutes les lumières au point touché.
    Color computeDirectLighting(const IPrimitive& primitive,
                                const HitInfo& info) const;

    // Pour les reflets (could) : génère le rayon réfléchi et rappelle computeColor(depth+1).
    Color computeReflection(const HitInfo& info, int depth) const;

    // Pour la réfraction (could) : génère le rayon réfracté et rappelle computeColor(depth+1).
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
    // Lit le .cfg, construit la scène via SceneBuilder.
    Scene parse(const std::string& filepath);

private:
    Camera      parseCamera(const libconfig::Setting& cfg);
    IPrimitive* parsePrimitive(const libconfig::Setting& cfg);
    ILight*     parseLight(const libconfig::Setting& cfg);

    // Construit la matrice de transformation depuis les champs
    // translate/rotate/scale du .cfg, puis wrappe la primitive dans un Transform.
    IPrimitive* applyTransforms(IPrimitive* primitive,
                                const libconfig::Setting& cfg);
};
```

---

## PPMWriter

```cpp
class PPMWriter {
public:
    // Format PPM P3 (ASCII).
    // En-tête : "P3\nwidth height\n255\n"
    // Ensuite : "r g b " par pixel, ligne par ligne.
    static void write(const std::string& filepath,
                      const std::vector<std::vector<Color>>& pixels);
};
```

---

## Relations entre les classes

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
       │     ├── computeReflection()   [depth > 0, could]
       │     └── computeRefraction()  [depth > 0, could]
       └─► PPMWriter.write()
```

---

## Extensibilité vers les should / could

| Fonctionnalité | Ce qu'il faut faire | Ce qui ne change pas |
|---|---|---|
| Cylinder, Cone (should) | Nouveau plugin `.so` | IPrimitive, Renderer, Scene |
| Rotation (should) | Transform + Matrix4x4 existent déjà | IPrimitive, primitives concrètes |
| Drop shadows (should) | DirectionalLight envoie shadowRay via `scene` | ILight signature déjà prévue |
| Phong (could) | Utilise `viewDir` déjà dans HitInfo | ILight signature déjà prévue |
| Reflection (could) | `computeReflection()` dans Renderer, `_maxDepth > 0` | IPrimitive, ILight |
| Refraction (could) | `computeRefraction()` dans Renderer | IPrimitive, ILight |
| Multithreading (could) | Renderer stateless → paralléliser `render()` par bandes | Tout le reste |
| Textures (could) | `getColor(point)` reçoit déjà le point | IPrimitive signature déjà prévue |
| Scale (could) | Matrix4x4 supporte déjà scale | Transform existe déjà |
| Multiple point lights (could) | Nouveau plugin `PointLight.so` | ILight, Renderer |

---

## Structure des fichiers

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
    ├── cylinder/      Cylinder.cpp         →  raytracer_cylinder.so   (should)
    ├── cone/          Cone.cpp             →  raytracer_cone.so       (should)
    ├── ambient/       AmbientLight.cpp     →  raytracer_ambient.so
    └── directional/   DirectionalLight.cpp →  raytracer_directional.so
```

---

## Ordre d'implémentation recommandé

1. `Point3D`, `Vector3D`, `Color` — aucune dépendance
2. `Ray`, `Rectangle3D`, `Matrix4x4` — dépendent de Vector3D/Point3D
3. `IPrimitive`, `ILight`, `HitInfo` — les contrats à poser avant tout
4. `Sphere` + `Plane` (plugins) — premières intersections testables
5. `Camera` + `Rectangle3D::pointAt` — génération des rayons
6. `PrimitiveList` (Composite) + `Transform` (Decorator)
7. `AmbientLight` + `DirectionalLight` (plugins)
8. `SceneBuilder` (Builder) + `PrimitiveFactory` + `LightFactory`
9. `SceneParser` (libconfig++)
10. `Renderer` (depth=0 d'abord) + `PPMWriter`
11. Test minimal : une sphère, une lumière ambiante, sortie PPM
12. Drop shadows : activer shadowRay dans `DirectionalLight::computeContribution`
13. Cylinder, Cone : nouveaux plugins, rien d'autre à toucher
14. Reflection/Refraction : passer `_maxDepth > 0` au Renderer
