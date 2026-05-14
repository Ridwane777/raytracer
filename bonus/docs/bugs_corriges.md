# Bugs corrigés — RayTracer

Ce document recense tous les bugs détectés et corrigés lors de la revue de code du projet, avec leur explication et les changements effectués.

---

## Bug 1 — `Matrix4x4::transformRay` : transformation du mauvais rayon

### Fichier
`src/math/Matrix4x4.cpp`

### Symptôme
Le `Transform` (Decorator) ne transformait pas correctement les rayons dans l'espace local des objets. Tous les objets wrappés par un `Transform` auraient eu un comportement d'intersection complètement erroné.

### Explication
Dans la méthode `transformRay`, un rayon vide `Ray r` était créé, puis c'est **ce rayon vide** qui était transformé au lieu du paramètre `ray` passé à la fonction.

### Code avant (cassé)
```cpp
Ray Matrix4x4::transformRay(const Ray& ray) const {
    Ray r;
    r.origin    = transformPoint(r.origin);    // ← transforme le rayon vide, pas ray !
    r.direction = transformVector(r.direction); // ← idem
    return r;
}
```

### Code après (corrigé)
```cpp
Ray Matrix4x4::transformRay(const Ray& ray) const {
    Ray r;
    r.origin    = transformPoint(ray.origin);             // ← ray (le paramètre)
    r.direction = transformVector(ray.direction).normalize(); // ← + normalize pour le scale
    return r;
}
```

### Pourquoi `.normalize()` en plus ?
Si la matrice contient un Scale (étirement), le vecteur direction peut ne plus avoir une longueur de 1 après transformation. Les calculs d'intersection supposent une direction normalisée, donc on la renormalise systématiquement.

---

## Bug 2 — `Transform::getNormal` : normales incorrectes lors d'un Scale

### Fichiers
`src/scene/Transform.cpp`, `src/math/Matrix4x4.hpp / .cpp`

### Symptôme
Lorsqu'une primitive subit un Scale non uniforme (ex: transformer une sphère en ovale), les normales retournées par `Transform` auraient été déformées. Les calculs de lumière et d'ombres auraient été visuellement faux (éclairage asymétrique là où il devrait être symétrique).

### Explication
Les vecteurs **normaux** ne se transforment pas comme les vecteurs ordinaires. En géométrie différentielle, une normale est un **covecteur** : si la transformation est la matrice $M$, il faut transformer la normale par **la transposée de l'inverse** de $M$, soit $(M^{-1})^T$.

Sans cette règle :
- Un Scale `(2, 1, 1)` sur l'axe X étire la surface horizontalement.
- En appliquant ce même Scale à la normale, elle s'allonge aussi sur X et n'est plus perpendiculaire à la surface.

Le projet possédait déjà `_worldToLocal` qui est $M^{-1}$. Il suffisait d'y ajouter `transpose()`.

### Code avant (cassé)
```cpp
Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _localToWorld.transformVector(localNormal).normalize(); // ← mauvaise matrice
}
```

### Code après (corrigé)
```cpp
Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _worldToLocal.transpose().transformVector(localNormal).normalize(); // ← (M⁻¹)ᵀ
}
```

### Méthode ajoutée : `Matrix4x4::transpose()`
```cpp
Matrix4x4 Matrix4x4::transpose() const {
    Matrix4x4 res;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            res.m[i][j] = m[j][i]; // échange lignes et colonnes
    return res;
}
```

> **Note :** Pour les rotations et translations pures (sans scale), les deux formules donnent le même résultat. Ce bug n'aurait été visible qu'en cas d'utilisation de `Matrix4x4::scale()`.

---

## Bug 3 — `SceneBuilder` : pointeur danglant (Dangling Pointer)

### Fichiers
`src/scene/SceneBuilder.cpp`, `src/scene/SceneBuilder.hpp`

### Symptôme
Crash ou comportement indéfini à l'exécution dès qu'une primitive est accédée par le Renderer, car la mémoire pointée avait déjà été libérée.

### Explication
`PrimitiveFactory::create()` retourne un `shared_ptr<IPrimitive>`. Ce `shared_ptr` **possède** la primitive : quand il est détruit, il appelle `destroyPrimitive()` puis `dlclose()` sur le plugin.

Dans le code original, le `shared_ptr` était détruit immédiatement après avoir ajouté le pointeur brut à la `PrimitiveList` :

```
1. shared_ptr créé → primitive en mémoire, ref_count = 1
2. primitive.get() → adresse brute stockée dans PrimitiveList
3. primitive.reset() → ref_count tombe à 0 → destroyPrimitive() + dlclose()
4. PrimitiveList garde l'ancienne adresse → POINTEUR DANGLANT 💥
```

### Schéma de propriété

```
AVANT (cassé) :
  BuiltScene
  └── PrimitiveList
        └── raw ptr ──→ [MÉMOIRE LIBÉRÉE 💥]

APRÈS (corrigé) :
  BuiltScene
  ├── PrimitiveList
  │     └── raw ptr ─────────────────┐
  └── ownedPrimitives (vector)       │
        └── shared_ptr ──────────→ [Sphere ✅]  (même adresse)
```

### Code avant (cassé)
```cpp
// Dans SceneBuilder::addSphere
auto primitive = _primitiveFactory.create(pluginPath("sphere"), params);
list.add(primitive.get());  // on donne l'adresse brute à la liste
(void)primitive.get();
primitive.reset();          // ← DÉTRUIT la primitive ! La liste pointe dans le vide.
```

### Code après (corrigé)

**`SceneBuilder.hpp`** : ajout de `ownedPrimitives` dans `BuiltScene`
```cpp
struct BuiltScene {
    Camera                                      camera;
    PrimitiveList                               primitives;
    std::vector<std::shared_ptr<IPrimitive>>    ownedPrimitives; // ← NOUVEAU : garde ownership
    std::vector<std::shared_ptr<ILight>>        lights;
    double ambientIntensity = 0.4;
    double diffuseIntensity = 0.6;
};
```

**`SceneBuilder.cpp`** : transfert du `shared_ptr` dans `ownedPrimitives`
```cpp
auto primitive = _primitiveFactory.create(pluginPath("sphere"), params);
scene.primitives.add(primitive.get());                     // adresse brute dans la liste
scene.ownedPrimitives.push_back(std::move(primitive));     // ownership transféré à BuiltScene
// → La primitive vivra aussi longtemps que le BuiltScene
```

### Règle d'or avec `dlopen`
Un plugin chargé via `dlopen` **doit rester en mémoire** aussi longtemps que des objets créés depuis ce plugin existent. `ownedPrimitives` garantit cela : quand le `BuiltScene` est détruit en fin de programme, les `shared_ptr` sont libérés dans l'ordre, et `dlclose()` est appelé proprement.

---

## Bug 4 — `Sphere::createPrimitive` : mauvais noms de paramètres

### Fichier
`plugins/sphere/Sphere.cpp`

### Symptôme
Crash `std::out_of_range` (exception de `params.at(...)`) au lancement, dès qu'une sphère est lue depuis le fichier `.cfg`.

### Explication
Le `SceneBuilder` envoie les paramètres sous les clés `"r"` (rayon), `"color_r"`, `"color_g"`, `"color_b"`.
Le `createPrimitive` lisait les clés `"radius"`, `"r"`, `"g"`, `"b"` qui n'existent pas dans la map → exception.

### Changement
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

## Bug 5 — `Plane::createPrimitive` : axe incorrect et mauvais noms de paramètres

### Fichier
`plugins/plane/Plane.cpp`

### Symptômes
1. Tous les plans étaient traités comme des plans de l'axe Z, quelle que soit la valeur configurée.
2. Crash `std::out_of_range` pour les paramètres de couleur.

### Explication — l'axe

Le `SceneBuilder` encode l'axe en `double` : `0.0` = X, `1.0` = Y, `2.0` = Z.
Le constructeur de `Plane` attend un `char` : `'X'`, `'Y'` ou `'Z'`.

L'ancien code faisait `static_cast<char>(params.at("axis"))`, ce qui donnait `char(0)`, `char(1)` ou `char(2)` (des caractères de contrôle ASCII), jamais les lettres `'X'`/`'Y'`/`'Z'`. La méthode `Plane::hits()` tombait donc toujours dans le `else` (axe Z).

### Explication — les couleurs
Même problème que pour la Sphere : les clés envoyées sont `"color_r/g/b"` mais le code lisait `"r/g/b"`.

### Changement
```diff
  extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
  {
-     char   axis     = static_cast<char>(params.at("axis")); // → char(0/1/2) ≠ 'X'/'Y'/'Z'
-     double position = params.at("position");
-     Color  color(params.at("r"), params.at("g"), params.at("b"));
+     double axisVal = params.at("axis");
+     char   axis    = 'Y'; // Y par défaut
+     if (axisVal == 0.0)      axis = 'X';
+     else if (axisVal == 2.0) axis = 'Z';
+     double position = params.at("position");
+     Color  color(params.at("color_r"), params.at("color_g"), params.at("color_b"));
      return new Plane(axis, position, color);
  }
```

---

## Bug 6 — `Camera::getScreen` : rotation ignorée

### Fichier
`src/scene/Camera.cpp`

### Symptôme
Changer la valeur de `rotation` de la caméra dans le fichier `.cfg` n'avait aucun effet : la caméra regardait toujours dans la direction `-Z` par défaut.

### Explication
Le plan écran (`Rectangle3D`) était construit avec des vecteurs hardcodés en axe X et Y, sans jamais appliquer l'attribut `rotation` de la caméra.

### Changement
```diff
  Rectangle3D Camera::getScreen() const {
      double ratio      = (double)width / (double)height;
      double halfHeight = tan((fov * M_PI / 180.0) / 2.0);
      double halfWidth  = ratio * halfHeight;

-     // Hardcodé en axe -Z
-     Point3D  origin      = Point3D(position.X - halfWidth, position.Y - halfHeight, position.Z - 1.0);
-     Vector3D bottom_side = Vector3D((2.0 * halfWidth), 0.0, 0.0);
-     Vector3D left_side   = Vector3D(0.0, (2.0 * halfHeight), 0.0);

+     // Matrice de rotation appliquée aux vecteurs du plan écran
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
