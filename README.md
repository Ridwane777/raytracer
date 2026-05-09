# RayTracer

Moteur de rendu 3D par lancer de rayons (Raytracing) développé en C++20. Ce projet génère des images au format `.ppm` à partir de fichiers de configuration de scènes via un système de plugins dynamiques (`.so`).

## Architecture & Design Patterns

Ce projet respecte les principes SOLID et utilise plusieurs Design Patterns incontournables :

- **Factory Pattern** (`PrimitiveFactory`, `LightFactory`) : Permet le chargement dynamique des plugins `.so`. Le cœur du programme n'a aucune connaissance des primitives ou lumières existantes.
- **Decorator Pattern** (`Transform`) : Utilisé pour les transformations spatiales. Au lieu de coder les positions en dur, chaque primitive est créée à l'origine (0,0,0) puis enveloppée dans un `Transform` qui applique une matrice de transformation 4x4.
- **Composite Pattern** (`PrimitiveList`) : Gère une collection d'objets `IPrimitive` comme un objet unique, permettant de lancer un rayon dans la scène entière en un seul appel de méthode `hits()`.
- **Builder Pattern** (`SceneBuilder`) : Construit l'objet `Scene` final étape par étape à partir des données extraites par le `SceneParser`.

---

## Fonctionnalités (Barème Ridwane777)

L'implémentation actuelle couvre toutes les exigences principales ainsi que de nombreux bonus mathématiques et architecturaux.

### ✅ MUST (Fonctionnalités de base)
- **Primitives** : Sphere, Plane (via plugins dynamiques)
- **Transformations** : Translation
- **Light** : Directional light, Ambient light
- **Material** : Flat color (Calcul des couleurs en RGB)
- **Interface** : Configuration des scènes via fichier texte (libconfig)
- **Output** : Sauvegarde au format image `.ppm` (P3)
- **Robustesse** : Gestion des erreurs sur la sortie standard d'erreur (`stderr`), usage `--help`.

### ✅ SHOULD (Fonctionnalités intermédiaires)
- **Primitives** : Cylinder, Cone (via plugins)
- **Light** : Drop shadows (Ombres portées réalistes via "Shadow Rays").

### 🏆 COULD (Bonus & Fonctionnalités Avancées)
- **Transformations : Transformation Matrix** : Le système utilise de véritables matrices 4x4 combinées et inversées pour gérer la position des objets (Pattern Decorator).
- **Transformations : Rotations** : L'algèbre complète de rotation (`rotationX`, `Y`, `Z`) est mathématiquement implémentée et prête à l'emploi dans le moteur matriciel.
- **Light : Multiple directional lights** : L'architecture supporte une liste illimitée de sources lumineuses additionnant leurs contributions.
- **Light : Point light** : Plugin de lumière ponctuelle implémenté (`plugins/point/PointLight`), agissant comme une ampoule avec sa propre position 3D et gérant parfaitement les ombres multiples.

---

## Compilation & Utilisation

### Prérequis
- Compilateur compatible `C++20`
- `cmake` (>= 3.17)
- `libconfig++` (`sudo dnf install libconfig-c++-devel` ou `sudo apt install libconfig++-dev`)

### Build

```bash
cmake -B build -S .
cmake --build build
```

### Lancement

```bash
# Générer le rendu de la scène de test
./raytracer scenes/test.cfg
```
L'image résultante sera sauvegardée sous le nom `output.ppm`.