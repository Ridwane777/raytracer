# Répartition des tâches — Raytracer

---

## Socle commun (tout le monde, en premier)

À finir avant de se séparer — tout le monde en dépend.

- `src/interfaces/IPrimitive.hpp`
- `src/interfaces/ILight.hpp`
- `src/interfaces/HitInfo.hpp`
- `src/math/Point3D`, `Vector3D`, `Color`, `Ray`, `Rectangle3D`, `Matrix4x4`

---

## Ridwane — Math, Camera & **Decorator**

**Design pattern : Decorator** — `Transform` enveloppe n'importe quelle primitive et lui applique une matrice (translation, rotation, scale) sans modifier la primitive elle-même.

- `src/math/Matrix4x4.cpp` — `inverse()`, `transformRay()`, `rotationX/Y/Z()`
- `src/scene/Camera.hpp / .cpp` — `generateRay(u, v)`
- `src/scene/Transform.hpp / .cpp` — **Decorator** : wrappe un `IPrimitive*` avec une `Matrix4x4`

---

## Darel — Primitives & **Composite**

**Design pattern : Composite** — `PrimitiveList` implémente `IPrimitive` et contient une liste de primitives. Le Renderer n'a qu'un seul `IPrimitive*` à appeler, sans savoir combien d'objets sont dans la scène.

- `plugins/sphere/Sphere.cpp` → `raytracer_sphere.so`
- `plugins/plane/Plane.cpp` → `raytracer_plane.so`
- `plugins/cylinder/Cylinder.cpp` → `raytracer_cylinder.so` _(should)_
- `plugins/cone/Cone.cpp` → `raytracer_cone.so` _(should)_
- `src/scene/PrimitiveList.hpp / .cpp` — **Composite**

---

## Shalom — Lumières & **Factory**

**Design pattern : Factory** — `LightFactory` charge les plugins lumière via `dlopen` et retourne un `ILight*` à partir d'un nom textuel (`"ambient"`, `"directional"`). Le reste du programme ne fait jamais `new AmbientLight` directement.

- `plugins/ambient/AmbientLight.cpp` → `raytracer_ambient.so`
- `plugins/directional/DirectionalLight.cpp` → `raytracer_directional.so`
- `src/factory/LightFactory.hpp / .cpp` — **Factory** : `dlopen` + `createLight`

---

## Raoul — Parser, Renderer & **Builder**

**Design pattern : Builder** — `SceneBuilder` construit la scène étape par étape (`setCamera()`, `addPrimitive()`, `addLight()`, `build()`). Le parser ne remplit jamais la `Scene` directement, ce qui le rend indépendant et testable.

- `src/factory/PrimitiveFactory.hpp / .cpp` — Factory : `dlopen` + `createPrimitive`
- `src/scene/Scene.hpp / .cpp`
- `src/scene/SceneBuilder.hpp / .cpp` — **Builder**
- `src/parser/SceneParser.hpp / .cpp` — libconfig++
- `src/renderer/Renderer.hpp / .cpp`
- `src/output/PPMWriter.hpp / .cpp`
- `src/main.cpp`
