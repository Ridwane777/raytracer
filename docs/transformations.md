# Transformations: Decorator Pattern and 4x4 Matrices

## The problem with the naive approach

### What does "translating" an object mean in a raytracer?

Translating means moving an object from point A to point B in the scene.

A naive raytracer does this by *hardcoding the object's position in its own data*.

That is what the code did **before** the refactoring. For the sphere, for example:

```cpp
// SceneBuilder sent this to the Sphere plugin:
params["x"] = 60.0;  // ← the position is stored in the sphere itself
params["y"] = 5.0;
params["z"] = 40.0;

// And in Sphere.cpp, hits() did:
double dx = ray.origin.X - _center.X;  // _center = (60, 5, 40)
double dy = ray.origin.Y - _center.Y;
double dz = ray.origin.Z - _center.Z;
```

Visually it worked. **Architecturally**, it was a dead end.

### Why is it bad practice?

**Reason 1: it is not a "transformation" in the mathematical sense**

The goal is real *transformations* (translation, rotation, scale...). A transformation is a mathematical operation applied to an object, independent of it. Here the sphere "knows" it is at (60, 5, 40). It was not *transformed*, it was *built* there. The distinction matters in software architecture.

**Reason 2: not extensible**

To rotate a sphere, the Sphere code would have to change. And the Cylinder. And the Cone. Every primitive would reimplement the same transformation logic, violating **DRY** (*Don't Repeat Yourself*) and the **Open/Closed** principle.

---

## The solution: Decorator pattern + 4x4 matrices

### The Decorator principle

Instead of modifying the primitive, we **wrap** it in an intermediate object that handles the transformation. This is the **Decorator pattern**:

- `Transform` implements the same `IPrimitive` interface as the Sphere
- It holds a reference to the inner primitive (the actual Sphere)
- `Transform::hits()` first transforms the ray, then delegates to `Sphere::hits()`
- The Sphere does not know it is inside a Transform

```
Scene
  └── Transform   (IPrimitive)
        ├── matrix = translation(60, 5, 40)
        └── Sphere(center=(0,0,0), r=25)   (IPrimitive)
```

### Why 4x4 matrices?

In 3D, all geometric transformations (translation, rotation, scale) can be represented as **4x4 matrices**. The 4th dimension is needed to express translation as a matrix multiplication.

The translation matrix for vector `(tx, ty, tz)`:

```
T = [1  0  0  tx]
    [0  1  0  ty]
    [0  0  1  tz]
    [0  0  0   1]
```

The main benefit: transformations **combine** by **multiplying** their matrices.
To "translate by (5,0,0) then rotate 45° around Y":

```cpp
Matrix4x4 M = Matrix4x4::rotationY(45.0) * Matrix4x4::translation(Vector3D(5, 0, 0));
```

The result is a single matrix that performs both operations in one call.

### How it works at runtime

The Transform trick: instead of **moving the object**, we **move the ray the opposite way**.

If the sphere is translated by `(60, 5, 40)` in world space, the ray is transformed by the **inverse** matrix `(-60, -5, -40)` to bring it into local space, where the sphere sits at the origin:

```
World ray  →  [Transform: applies the inverse]  →  Local ray  →  Sphere.hits()
                    (translation by -60, -5, -40)       ↑
                                                  Sphere at (0,0,0)
```

The normal computed in local space is then transformed back to world space with the **transpose of the inverse** of the matrix (the standard formula for normals).

```cpp
// In Transform.cpp
std::optional<double> Transform::hits(const Ray& ray) const {
    Ray localRay = _worldToLocal.transformRay(ray);   // ray → local space
    return _wrapped->hits(localRay);                  // delegate to the primitive
}

Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _worldToLocal.transpose().transformVector(localNormal).normalize();
}

// New for materials (transparency / refraction / reflection):
// Transform forwards these queries to the inner object unchanged.
double Transform::getOpacity() const { return _wrapped->getOpacity(); }
double Transform::getRefractiveIndex() const { return _wrapped->getRefractiveIndex(); }
double Transform::getReflectivity() const { return _wrapped->getReflectivity(); }
```

---

## What changed in the code

### `Transform.hpp` / `Transform.cpp`

The class already existed but took an `IPrimitive*` (raw pointer = potential leak or dangling pointer if the `.so` plugin is unloaded before the primitive).

```cpp
// BEFORE: dangerous raw pointer
Transform(IPrimitive* primitive, const Matrix4x4& transform);
IPrimitive* _wrapped;

// AFTER: shared_ptr = shared ownership, no leak
Transform(std::shared_ptr<IPrimitive> primitive, const Matrix4x4& transform);
std::shared_ptr<IPrimitive> _wrapped;
```

The `shared_ptr` keeps the wrapped primitive alive as long as the Transform exists, even if the factory released it elsewhere.

### `SceneBuilder.cpp`

This is where the main change happens. Each primitive now follows a two-step protocol:

```cpp
// BEFORE: the Sphere knows its final position
params["x"] = 60.0;
params["y"] = 5.0;
params["z"] = 40.0;
auto sphere = factory.create("sphere", params);
scene.addPrimitive(sphere);

// AFTER: the Sphere is at the origin, Transform handles the move
params["x"] = 0.0;    // ← always at the origin
params["y"] = 0.0;
params["z"] = 0.0;
auto base = factory.create("sphere", params);

// buildTransformMatrix combines translation, rotation, scale and shear
Matrix4x4 tr = buildTransformMatrix(cfg, 60.0, 5.0, 40.0);
auto translated = std::make_shared<Transform>(base, tr);
scene.addPrimitive(translated);  // add the Transform, not the Sphere directly
```

**For the plane**, the reasoning differs slightly because a plane is defined by an axis and a position on that axis. It is created at `position = 0` and then translated only along its axis:

```cpp
// Y plane at position = -3 → translate by (0, -3, 0)
double ty = (axisVal == 1.0) ? cfg.position : 0.0;
Matrix4x4 tr = buildTransformMatrix(cfg, 0.0, ty, 0.0);
auto translated = std::make_shared<Transform>(basePlane, tr);
```

---

## Design patterns used

| Pattern | Where | Role |
|---|---|---|
| **Decorator** | `Transform` wrapping `IPrimitive` | Adds a transformation without modifying the wrapped class |
| **Factory** | `PrimitiveFactory`, `LightFactory` | Instantiates plugins from `.so` files without knowing the concrete classes |
| **Builder** | `SceneBuilder` | Builds the complex `Scene` object step by step from a `SceneConfig` |
| **Composite** | `PrimitiveList` | Treats a group of primitives as a single `IPrimitive` |

---

## One-sentence summary

> *"Every primitive is built at the origin. To place it in the scene, it is wrapped in a `Transform` (Decorator) that stores a 4×4 matrix and its inverse. At runtime, the Transform applies the inverse matrix to the incoming ray to bring it into the primitive's local space before delegating the intersection test, then transforms the normal back to world space."*