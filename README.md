# RayTracer

Moteur de rendu 3D par lancer de rayons (Raytracing) développé en C++20. Ce projet génère des images au format `.ppm` à partir de fichiers de configuration de scènes via un système de plugins dynamiques (`.so`).

## Architecture & Design Patterns

Ce projet respecte les principes SOLID et utilise plusieurs Design Patterns incontournables :

- **Factory Pattern** (`PrimitiveFactory`, `LightFactory`) : Permet le chargement dynamique des plugins `.so`. Le cœur du programme n'a aucune connaissance des primitives ou lumières existantes.
- **Decorator Pattern** (`Transform`) : Utilisé pour les transformations spatiales. Au lieu de coder les positions en dur, chaque primitive est créée à l'origine (0,0,0) puis enveloppée dans un `Transform` qui applique une matrice de transformation 4x4.
- **Composite Pattern** (`PrimitiveList`) : Gère une collection d'objets `IPrimitive` comme un objet unique, permettant de lancer un rayon dans la scène entière en un seul appel de méthode `hits()`.
- **Builder Pattern** (`SceneBuilder`) : Construit l'objet `Scene` final étape par étape à partir des données extraites par le `SceneParser`.

---

## Fonctionnalités

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
- **Transformations : Rotations** : L'algèbre complète de rotation (`rotationX`, `Y`, `Z`) est mathématiquement implémentée et prête à l'emploi via le fichier de configuration.
- **Light** : Drop shadows (Ombres portées réalistes via "Shadow Rays").

### 🏆 COULD (Bonus & Fonctionnalités Avancées)
- **Primitives : Triangle** : Support des polygones plats à trois sommets (plugin dynamique `triangle.so`) utilisant l'intersection plan-polygone.
- **Primitives : Torus** : Support de la primitive complexe de tore (plugin dynamique `torus.so`) par résolution analytique d'une équation polynomiale du 4ème degré.
- **Primitives : .OBJ file** : Chargement de fichiers de modèles 3D complexes au format Wavefront `.obj` avec triangulation automatique à la volée.
- **Transformations : Transformation Matrix** : Le système utilise de véritables matrices 4x4 combinées et inversées pour gérer de façon transparente toutes les opérations de positionnement (Pattern Decorator).
- **Transformations : Scale & Shear** : Déformation d'échelle et cisaillement (shear) implémentés directement dans le moteur matriciel, applicables à n'importe quelle primitive sans retoucher son code.
- **Light : Multiple lights** : L'architecture supporte une liste illimitée de sources lumineuses (ambiance, directionnelles, ponctuelles) additionnant leurs contributions.
- **Light : Point light** : Plugin de lumière ponctuelle (`plugins/point/PointLight`), agissant comme une ampoule locale gérant la décroissance lumineuse et les ombres multiples.
- **Light : Colored light** : Support spectral complet (RGB). La couleur finale est le produit du spectre de l'objet et de celui de la lumière qui l'éclaire.
- **Material : Transparency** : L'opacité des objets permet le passage du rayon au travers des primitives (mélange Alpha Blending).
- **Material : Refraction** : Implémentation physique complète de la Loi de Snell-Descartes avec indices de réfraction et gestion de la réflexion totale interne (ex: verre, eau).
- **Material : Reflection** : Effet miroir parfait avec limitation de rebonds via `_maxDepth` dans le moteur récursif.
- **Render : Antialiasing (Supersampling)** : Lissage des bords par lancer de plusieurs rayons par pixel avec décalages sub-pixels aléatoires.
- **Light : Phong reflection model** : Modèle d'illumination complet incluant les composantes ambiante, diffuse et spéculaire.

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