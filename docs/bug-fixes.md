# Bug Fixes - RayTracer

This document lists every bug found and fixed during the project's code review, with an explanation and the changes made.

---

## Bug 1 - `Matrix4x4::transformRay`: transforming the wrong ray

### File
`src/math/Matrix4x4.cpp`

### Symptom
The `Transform` (Decorator) did not correctly transform rays into object local space. Every object wrapped in a `Transform` would have had completely wrong intersection behavior.

### Explanation
In `transformRay`, an empty `Ray r` was created, and **that empty ray** was transformed instead of the `ray` parameter passed to the function.

### Before (broken)
```cpp
Ray Matrix4x4::transformRay(const Ray& ray) const {
    Ray r;
    r.origin    = transformPoint(r.origin);    // ← transforms the empty ray, not ray!
    r.direction = transformVector(r.direction); // ← same
    return r;
}
```

### After (fixed)
```cpp
Ray Matrix4x4::transformRay(const Ray& ray) const {
    Ray r;
    r.origin    = transformPoint(ray.origin);             // ← ray (the parameter)
    r.direction = transformVector(ray.direction).normalize(); // ← + normalize for scale
    return r;
}
```

### Why the extra `.normalize()`?
If the matrix contains a Scale (stretch), the direction vector may no longer have length 1 after transformation. Intersection math assumes a normalized direction, so it is always renormalized.

---

## Bug 2 - `Transform::getNormal`: wrong normals under Scale

### Files
`src/scene/Transform.cpp`, `src/math/Matrix4x4.hpp / .cpp`

### Symptom
When a primitive undergoes a non-uniform Scale (e.g. turning a sphere into an ellipsoid), the normals returned by `Transform` would have been distorted. Lighting and shadows would have looked wrong (asymmetric shading where it should be symmetric).

### Explanation
**Normal** vectors do not transform like ordinary vectors. In differential geometry, a normal is a **covector**: if the transformation is matrix $M$, the normal must be transformed by **the transpose of the inverse** of $M$, i.e. $(M^{-1})^T$.

Without this rule:
- A Scale `(2, 1, 1)` on the X axis stretches the surface horizontally.
- Applying the same Scale to the normal also stretches it along X, so it is no longer perpendicular to the surface.

The project already had `_worldToLocal`, which is $M^{-1}$. Adding `transpose()` was enough.

### Before (broken)
```cpp
Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _localToWorld.transformVector(localNormal).normalize(); // ← wrong matrix
}
```

### After (fixed)
```cpp
Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _worldToLocal.transpose().transformVector(localNormal).normalize(); // ← (M⁻¹)ᵀ
}
```

### Added method: `Matrix4x4::transpose()`
```cpp
Matrix4x4 Matrix4x4::transpose() const {
    Matrix4x4 res;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            res.m[i][j] = m[j][i]; // swap rows and columns
    return res;
}
```

> **Note:** For pure rotations and translations (no scale), both formulas give the same result. This bug would only have shown up when using `Matrix4x4::scale()`.

---

## Bug 3 - `SceneBuilder`: dangling pointer

### Files
`src/scene/SceneBuilder.cpp`, `src/scene/SceneBuilder.hpp`

### Symptom
Crash or undefined behavior at runtime as soon as the Renderer accessed a primitive, because the pointed-to memory had already been freed.

### Explanation
`PrimitiveFactory::create()` returns a `shared_ptr<IPrimitive>`. This `shared_ptr` **owns** the primitive: when destroyed, it calls `destroyPrimitive()` then `dlclose()` on the plugin.

In the original code, the `shared_ptr` was destroyed right after adding the raw pointer to the `PrimitiveList`:

```
1. shared_ptr created → primitive in memory, ref_count = 1
2. primitive.get() → raw address stored in PrimitiveList
3. primitive.reset() → ref_count drops to 0 → destroyPrimitive() + dlclose()
4. PrimitiveList keeps the old address → DANGLING POINTER 💥
```

### Ownership diagram

```
BEFORE (broken):
  BuiltScene
  └── PrimitiveList
        └── raw ptr ──→ [FREED MEMORY 💥]

AFTER (fixed):
  BuiltScene
  ├── PrimitiveList
  │     └── raw ptr ─────────────────┐
  └── ownedPrimitives (vector)       │
        └── shared_ptr ──────────→ [Sphere ✅]  (same address)
```

### Before (broken)
```cpp
// In SceneBuilder::addSphere
auto primitive = _primitiveFactory.create(pluginPath("sphere"), params);
list.add(primitive.get());  // give the raw address to the list
(void)primitive.get();
primitive.reset();          // ← DESTROYS the primitive! The list points to nothing.
```

### After (fixed)

**`SceneBuilder.hpp`**: add `ownedPrimitives` to `BuiltScene`
```cpp
struct BuiltScene {
    Camera                                      camera;
    PrimitiveList                               primitives;
    std::vector<std::shared_ptr<IPrimitive>>    ownedPrimitives; // ← NEW: keeps ownership
    std::vector<std::shared_ptr<ILight>>        lights;
    double ambientIntensity = 0.4;
    double diffuseIntensity = 0.6;
};
```

**`SceneBuilder.cpp`**: move the `shared_ptr` into `ownedPrimitives`
```cpp
auto primitive = _primitiveFactory.create(pluginPath("sphere"), params);
scene.primitives.add(primitive.get());                     // raw address in the list
scene.ownedPrimitives.push_back(std::move(primitive));     // ownership moved to BuiltScene
// → The primitive lives as long as the BuiltScene
```

### Golden rule with `dlopen`
A plugin loaded with `dlopen` **must stay in memory** as long as objects created from it exist. `ownedPrimitives` guarantees this: when the `BuiltScene` is destroyed at program exit, the `shared_ptr`s are released in order and `dlclose()` is called cleanly.

---

## Bug 4 - `Sphere::createPrimitive`: wrong parameter names

### File
`plugins/sphere/Sphere.cpp`

### Symptom
`std::out_of_range` crash (exception from `params.at(...)`) at startup, as soon as a sphere is read from the `.cfg` file.

### Explanation
`SceneBuilder` sends parameters under the keys `"r"` (radius), `"color_r"`, `"color_g"`, `"color_b"`.
`createPrimitive` read the keys `"radius"`, `"r"`, `"g"`, `"b"`, which do not exist in the map → exception.

### Change
```diff
  extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
  {
      Point3D center(params.at("x"), params.at("y"), params.at("z"));
-     double  radius = params.at("radius");
-     Color   color(params.at("r"), params.at("g"), params.at("b"));
+     double  radius = params.at("r");
+     Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));
      return new Sphere(center, radius, color);
  }
```

---

## Bug 5 - `Plane::createPrimitive`: wrong axis and wrong parameter names

### File
`plugins/plane/Plane.cpp`

### Symptoms
1. Every plane was treated as a Z-axis plane, regardless of the configured value.
2. `std::out_of_range` crash for the color parameters.

### Explanation - the axis

`SceneBuilder` encodes the axis as a `double`: `0.0` = X, `1.0` = Y, `2.0` = Z.
The `Plane` constructor expects a `char`: `'X'`, `'Y'` or `'Z'`.

The old code did `static_cast<char>(params.at("axis"))`, which yielded `char(0)`, `char(1)` or `char(2)` (ASCII control characters), never the letters `'X'`/`'Y'`/`'Z'`. `Plane::hits()` therefore always fell into the `else` branch (Z axis).

### Explanation - the colors
Same problem as Sphere: the keys sent are `"color_r/g/b"` but the code read `"r/g/b"`.

### Change
```diff
  extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
  {
-     char   axis     = static_cast<char>(params.at("axis")); // → char(0/1/2) ≠ 'X'/'Y'/'Z'
-     double position = params.at("position");
-     Color  color(params.at("r"), params.at("g"), params.at("b"));
+     double axisVal = params.at("axis");
+     char   axis    = 'Y'; // Y by default
+     if (axisVal == 0.0)      axis = 'X';
+     else if (axisVal == 2.0) axis = 'Z';
+     double position = params.at("position");
+     Color  color(params.at("color_r"), params.at("color_g"), params.at("color_b"));
      return new Plane(axis, position, color);
  }
```

---

## Bug 6 - `Camera::getScreen`: rotation ignored

### File
`src/scene/Camera.cpp`

### Symptom
Changing the camera `rotation` in the `.cfg` file had no effect: the camera always looked along the default `-Z` direction.

### Explanation
The screen plane (`Rectangle3D`) was built with vectors hardcoded along the X and Y axes, never applying the camera's `rotation` attribute.

### Change
```diff
  Rectangle3D Camera::getScreen() const {
      double ratio      = (double)width / (double)height;
      double halfHeight = tan((fov * M_PI / 180.0) / 2.0);
      double halfWidth  = ratio * halfHeight;

-     // Hardcoded along -Z
-     Point3D  origin      = Point3D(position.X - halfWidth, position.Y - halfHeight, position.Z - 1.0);
-     Vector3D bottom_side = Vector3D((2.0 * halfWidth), 0.0, 0.0);
-     Vector3D left_side   = Vector3D(0.0, (2.0 * halfHeight), 0.0);

+     // Rotation matrix applied to the screen plane vectors
+     Matrix4x4 rotMatrix = Matrix4x4::rotationX(rotation.X) *
+                           Matrix4x4::rotationY(rotation.Y) *
+                           Matrix4x4::rotationZ(rotation.Z);
+
+     Vector3D localOriginVec(-halfWidth, -halfHeight, -1.0);
+     Vector3D bottom_side = rotMatrix.transformVector(Vector3D(2.0 * halfWidth, 0.0, 0.0));
+     Vector3D left_side   = rotMatrix.transformVector(Vector3D(0.0, 2.0 * halfHeight, 0.0));
+     Point3D  origin      = position + rotMatrix.transformVector(localOriginVec);

      return Rectangle3D(origin, bottom_side, left_side);
  }
```
