# Transformations — Design Pattern Decorator et Matrices 4x4

## Le problème de l'approche naïve

### Qu'est-ce que "translater" un objet dans un raytracer ?

Translater = déplacer un objet d'un point A vers un point B dans la scène.

Dans un raytracer naïf, on traduit ça par : *"on code la position de l'objet directement dans ses données"*.

C'est ce que le code faisait **avant** le refactoring. Par exemple pour la sphère :

```cpp
// SceneBuilder envoyait ça au plugin Sphere :
params["x"] = 60.0;  // ← la position est codée dans la sphère elle-même
params["y"] = 5.0;
params["z"] = 40.0;

// Et dans Sphere.cpp, hits() faisait :
double dx = ray.origin.X - _center.X;  // _center = (60, 5, 40)
double dy = ray.origin.Y - _center.Y;
double dz = ray.origin.Z - _center.Z;
```

Visuellement, ça marchait. Mais **architecturalement**, c'est une impasse.

### Pourquoi c'est une mauvaise pratique ?

**Raison 1 — Ce n'est pas une "transformation" au sens mathématique**

Le sujet exige des *transformations* (translation, rotation, scale...). Une transformation est une opération mathématique appliquée à un objet indépendant de celui-ci. Mais là, la sphère "sait" qu'elle est à (60, 5, 40). Elle n'a pas été *transformée*, elle a été *construite* à cet endroit. La nuance est fondamentale en architecture logicielle.

**Raison 2 — Pas extensible**

Si on veut appliquer une **rotation** à une sphère, il faut modifier le code de la Sphère. Et du Cylindre. Et du Cône. Chaque primitive doit réimplémenter la même logique de transformation. C'est une violation du principe **DRY** (*Don't Repeat Yourself*) et du principe **Open/Closed**.

---

## La solution : Design Pattern Decorator + Matrices 4x4

### Le principe du Decorator

Au lieu de modifier la primitive, on l'**enveloppe** dans un objet intermédiaire qui gère la transformation. C'est le **Design Pattern Decorator** :

- `Transform` implémente la même interface `IPrimitive` que la Sphère
- Il possède une référence vers la primitive interne (la vraie Sphère)
- Quand on appelle `Transform::hits()`, il transforme d'abord le rayon, puis délègue à `Sphere::hits()`
- La Sphère ne sait pas qu'elle est dans un Transform

```
Scene
  └── Transform   (IPrimitive)
        ├── matrix = translation(60, 5, 40)
        └── Sphere(center=(0,0,0), r=25)   (IPrimitive)
```

### Pourquoi des matrices 4x4 ?

En 3D, toutes les transformations géométriques (translation, rotation, échelle) peuvent être représentées par des **matrices 4x4**. On a besoin de 4 dimensions (et non 3) pour pouvoir exprimer la translation sous forme de multiplication matricielle.

La matrice de translation par le vecteur `(tx, ty, tz)` :

```
T = [1  0  0  tx]
    [0  1  0  ty]
    [0  0  1  tz]
    [0  0  0   1]
```

L'avantage majeur : on peut **combiner** des transformations en **multipliant** leurs matrices.
Pour "translater de (5,0,0) puis pivoter de 45° autour de Y" :

```cpp
Matrix4x4 M = Matrix4x4::rotationY(45.0) * Matrix4x4::translation(Vector3D(5, 0, 0));
```

Et on obtient une seule matrice qui fait les deux opérations en un seul appel.

### Comment ça fonctionne à l'exécution

L'astuce du Transform : au lieu de **déplacer l'objet**, on **déplace le rayon dans le sens inverse**.

Si la sphère est translatée de `(60, 5, 40)` en monde, on transforme le rayon avec la matrice **inverse** `(-60, -5, -40)` pour ramener le rayon dans l'espace local où la sphère est à l'origine :

```
Rayon monde  →  [Transform : applique l'inverse]  →  Rayon local  →  Sphere.hits()
                    (translation de -60, -5, -40)       ↑
                                                  Sphère à (0,0,0)
```

La normale calculée dans l'espace local est ensuite retransformée vers l'espace monde via la **transposée de l'inverse** de la matrice (formule mathématique standard pour les normales).

```cpp
// Dans Transform.cpp
std::optional<double> Transform::hits(const Ray& ray) const {
    Ray localRay = _worldToLocal.transformRay(ray);   // rayon → espace local
    return _wrapped->hits(localRay);                  // délègue à la primitive
}

Vector3D Transform::getNormal(const Point3D& point) const {
    Point3D localPoint = _worldToLocal.transformPoint(point);
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    return _worldToLocal.transpose().transformVector(localNormal).normalize();
}
```

---

## Ce qui a été changé dans le code

### `Transform.hpp` / `Transform.cpp`

La classe existait déjà mais prenait un `IPrimitive*` (raw pointer = fuite mémoire potentielle si le plugin `.so` se décharge avant la primitive).

```cpp
// AVANT — raw pointer dangereux
Transform(IPrimitive* primitive, const Matrix4x4& transform);
IPrimitive* _wrapped;

// APRÈS — shared_ptr = propriété partagée, pas de fuite
Transform(std::shared_ptr<IPrimitive> primitive, const Matrix4x4& transform);
std::shared_ptr<IPrimitive> _wrapped;
```

Le `shared_ptr` garantit que la primitive encapsulée reste en vie aussi longtemps que le Transform existe, même si la factory l'a libéré par ailleurs.

### `SceneBuilder.cpp`

C'est là que se passe le changement principal. Pour chaque primitive, on suit maintenant ce protocole en deux étapes :

```cpp
// AVANT : la Sphère connaît sa position finale
params["x"] = 60.0;
params["y"] = 5.0;
params["z"] = 40.0;
auto sphere = factory.create("sphere", params);
scene.addPrimitive(sphere);

// APRÈS : la Sphère est à l'origine, Transform s'occupe du déplacement
params["x"] = 0.0;    // ← toujours à l'origine
params["y"] = 0.0;
params["z"] = 0.0;
auto base = factory.create("sphere", params);

// Le Transform stocke la matrice de translation ET son inverse
auto translated = std::make_shared<Transform>(
    base,
    Matrix4x4::translation(Vector3D(60.0, 5.0, 40.0))
);
scene.addPrimitive(translated);  // on ajoute le Transform, pas la Sphère directement
```

**Pour le plan**, le raisonnement est légèrement différent car un plan est défini par un axe et une position sur cet axe. On le crée à `position = 0` puis on translate uniquement le long de son axe :

```cpp
// Plan Y à position = -3 → translate de (0, -3, 0)
double ty = (axisVal == 1.0) ? cfg.position : 0.0;
auto translated = std::make_shared<Transform>(
    basePlane,
    Matrix4x4::translation(Vector3D(0.0, ty, 0.0))
);
```

---

## Design patterns utilisés

| Pattern | Où | Rôle |
|---|---|---|
| **Decorator** | `Transform` wrappant `IPrimitive` | Ajoute une transformation sans modifier la classe enveloppée |
| **Factory** | `PrimitiveFactory`, `LightFactory` | Instancie les plugins depuis les `.so` sans connaître les classes concrètes |
| **Builder** | `SceneBuilder` | Construit l'objet `Scene` complexe étape par étape depuis un `SceneConfig` |
| **Composite** | `PrimitiveList` | Traite un groupe de primitives comme une seule `IPrimitive` |

---

## Résumé en une phrase (pour la soutenance)

> *"Chaque primitive est construite à l'origine. Pour la positionner dans la scène, on l'encapsule dans un `Transform` (Decorator) qui stocke une matrice 4×4 et son inverse. À l'exécution, le Transform applique l'inverse de sa matrice au rayon entrant pour le ramener dans l'espace local de la primitive avant de déléguer le calcul d'intersection, puis retransforme la normale vers l'espace monde."*
