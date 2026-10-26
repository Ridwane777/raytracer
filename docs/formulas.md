# Raytracer Math Formulas

This document gathers the math formulas used in the core architecture and in the advanced features (primitives, lights, shadows, etc.).

---

## 1. Core formulas

### A. Vectors and 3D geometry (`Vector3D`)
* **Norm (vector length)**:
  `||V|| = sqrt(x² + y² + z²)`
* **Normalization (unit-length vector)**:
  `V_norm = V / ||V||`
* **Dot product**: measures how much two vectors point the same way. A result of 0 means they are perpendicular.
  `A . B = (A.x * B.x) + (A.y * B.y) + (A.z * B.z)`
* **Cross product**: gives a vector perpendicular to two others.
  `A x B = (A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y*B.x)`

### B. The ray (`Ray`)
* **Parametric ray equation**: a point `P` on the ray at distance `t`.
  `P(t) = Origin + t * Direction`

### C. Camera and screen plane (`Camera`)
* **Screen plane size from the FOV (field of view)**:
  `half_height = tan((FOV * PI / 180) / 2)`
  `half_width = half_height * (screen_width / screen_height)`
* **Ray generation for pixel (u, v)**, with u and v in [0, 1]:
  `TargetPoint = ScreenOrigin + u * BottomVector + v * LeftVector`
  `RayDirection = normalize(TargetPoint - CameraPosition)`

### D. Matrices and transformations (`Matrix4x4`, `Transform`)
* **Transforming a point / vector**: 4x4 matrix times 4x1 vector.
* **Transforming a normal** (keeps perpendicularity under non-uniform *scale*):
  `WorldNormal = Transpose(Inverse(TransformMatrix)) * LocalNormal`

---

## 2. Advanced feature formulas

### A. Geometric intersections (primitives)
The goal is to find the smallest `t > 0` such that `Ray(t) = object surface`.

**Sphere:**
Solve the quadratic `at² + bt + c = 0`:
* `L = RayOrigin - SphereCenter`
* `a = RayDirection . RayDirection` (equals 1 if the direction is normalized)
* `b = 2 * (RayDirection . L)`
* `c = (L . L) - SphereRadius²`
* **Discriminant**: `Δ = b² - 4ac`
  * If `Δ < 0`: no intersection.
  * If `Δ >= 0`: `t1 = (-b - sqrt(Δ)) / 2a` and `t2 = (-b + sqrt(Δ)) / 2a`. Keep the smallest `t > 0`.

**Plane:**
For a plane with normal `N` passing through point `A`:
* `t = ((A - RayOrigin) . N) / (RayDirection . N)`
* If `(RayDirection . N) == 0`: the ray is parallel to the plane (no intersection).

**Cylinder and cone:**
Both are also solved with a quadratic (`at² + bt + c = 0`):
* **Cylinder** (Y axis):
  * `a = Direction.x² + Direction.z²`
  * `b = 2 * (Direction.x * (Origin.x - Center.x) + Direction.z * (Origin.z - Center.z))`
  * `c = (Origin.x - Center.x)² + (Origin.z - Center.z)² - Radius²`
* **Cone** (Y axis, apex S):
  * `k = tan(Angle)²`
  * `a = Direction.x² + Direction.z² - k * Direction.y²`
  * `b = 2 * (Direction.x*(Orig.x - S.x) + Direction.z*(Orig.z - S.z) - k*Direction.y*(Orig.y - S.y))`
  * `c = (Orig.x - S.x)² + (Orig.z - S.z)² - k*(Orig.y - S.y)²`

---

### B. Lighting

For each light, its contribution is added to the pixel color.

**Ambient light (implemented):**
Residual light that reaches every point uniformly.
* `FinalColor = AmbientIntensity * ObjectColor`

**Diffuse light (Lambert's law) (implemented):**
Brightness depends on the angle between the light and the normal.
* `DiffuseIntensity = Max(0, Normal . LightDirection)`
* `Color = ObjectColor * LightColor * DiffuseIntensity`
*(Note: the light direction must point **toward** the light source for the dot product.)*

**Specular light (shiny highlight, Phong model) (implemented):**
The highlight of the sun on a shiny apple.
* **Reflection vector R**: `R = 2 * (Normal . LightDirection) * Normal - LightDirection`
* `SpecularIntensity = Max(0, View . R) ^ Shininess` (View is the direction to the camera; Shininess is a constant such as 32 or 64).

**Drop shadows (implemented):**
Before adding a light's color, a shadow ray is cast from the hit point toward the light source.
* If `PrimitiveList->hits(ShadowRay)` returns a hit, an object blocks the light -> that light's contribution is 0 (black).

---

### C. Reflection and refraction (implemented, recursive engine)

**Reflection (mirror):**
The ray bounces off the object and travels back into the scene.
* **New direction**: `R = RayDirection - 2 * (RayDirection . Normal) * Normal`
* `Renderer::computeColor()` is called again with this new ray.

**Refraction (glass / water, Snell's law):**
Light passes through the object and bends according to a refractive index `n` (e.g. air = 1.0, glass = 1.5).
* Index ratio: `r = n1 / n2`
* `cosI = -(Normal . RayDirection)`
* `sinT² = r² * (1 - cosI²)`
* If `sinT² > 1`: total internal reflection (the surface acts as a mirror).
* Otherwise, **new direction**: `T = r * RayDirection + (r * cosI - sqrt(1 - sinT²)) * Normal`
* **Final blend (alpha blending)**: `Color = (BaseColor * (1 - reflectivity)) + (ReflectedColor * reflectivity)`

---

### D. Triangles and 3D models

**Möller-Trumbore algorithm (ray-triangle intersection):**
Computes the intersection quickly without precomputing the triangle's plane.
For a triangle with vertices `A`, `B`, `C`:
1. **Edges**: `E1 = B - A`, `E2 = C - A`
2. **Vector P**: `P = RayDirection x E2`
3. **Determinant**: `det = E1 . P`
   * If `-EPSILON < det < EPSILON`, the ray is parallel to the triangle.
4. **Vector T**: `T = RayOrigin - A`
5. **Barycentric coordinate U**: `u = (T . P) / det`. If `u < 0` or `u > 1`, the ray misses.
6. **Vector Q**: `Q = T x E1`
7. **Barycentric coordinate V**: `v = (RayDirection . Q) / det`. If `v < 0` or `u + v > 1`, the ray misses.
8. **Distance t**: `t = (E2 . Q) / det`. If `t > EPSILON`, there is an intersection.
