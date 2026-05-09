# Bilan des Bonus (Coulds) - Raytracer

Ce fichier recense l'état d'avancement des fonctionnalités bonus (Coulds) telles que définies dans le barème officiel.

## 🏆 Bonus DÉJÀ VALIDÉS (Total : 6.5 points)

Notre architecture actuelle permet de cocher ces bonus sans écrire une seule ligne de code supplémentaire :

| Fonctionnalité | Points | Justification technique dans le code |
|---|---|---|
| **Limited cylinder** | `+ 0.5` | La fonction `hits` du cylindre limite mathématiquement les intersections via la propriété `height`. |
| **Limited cone** | `+ 0.5` | Le cône coupe également sa hauteur mathématiquement dans le plugin `cone.so`. |
| **Transformation matrix** | `+ 2.0` | Utilisation du Design Pattern *Decorator* (`Transform.cpp`) et de la classe `Matrix4x4` pour appliquer des matrices réelles et leurs inverses aux rayons de la scène. |
| **Multiple directional lights** | `+ 0.5` | Le parser et le moteur gèrent une liste `std::vector` illimitée pour additionner les contributions lumineuses. |
| **Multiple point lights** | `+ 0.5` | Plugin `point.so` fonctionnel avec lancer de rayon d'ombre (Shadow Rays) et gestion multi-lumières. |
| **Multithreading** | `+ 1.0` | Découpage du rendu en blocs et calcul parallèle sur tous les cœurs du CPU via `std::thread`. |
| **Progress Bar** | `N/A` | Animation fluide de progression en pourcentage affichée sur `stderr` durant le rendu. |
| **Antialiasing supersampling** | `+ 0.5` | Lancer de plusieurs rayons par pixel avec décalage aléatoire pour lisser les bords des objets. |
| **Scale** | `+ 0.5` | Ajout d'une matrice d'échelle `Matrix4x4::scale()` applicable dynamiquement à toute primitive depuis la configuration. |
| **Shear** | `+ 0.5` | Implémentation de `Matrix4x4::shear(xy, xz, ...)` et application en cascade dans le `Transform` decorator. |

---

## 🎯 Bonus rapides à ajouter (Discussion)

Voici une sélection des bonus les plus rapides à intégrer avec l'architecture actuelle (moins de 20 minutes par tâche) :

### 1. Optimisations & Interface
*(Plus de tâches rapides dans cette catégorie)*

### 2. Rendering
*(Plus de tâches rapides dans cette catégorie)*

### 3. Mathématiques & Lumières
*(Scale et Shear ont été ajoutés !)*
* **Colored light (+0.5 pt)** : Facile. Ajouter un vecteur `Color` aux lumières et multiplier avec la couleur de la primitive.
* **Transparency (+0.5 pt)** : Facile. Rendu de verre plat sans réfraction, via calcul du rayon passant au travers de l'objet.

### 4. Primitives & Modèles 3D (Objectif en cours)
* **Triangles (+1.0 pt)** : Implémentation de l'algorithme de Möller–Trumbore. Indispensable pour la suite.
* **.OBJ file (+1.0 pt)** : Création d'un parser pour lire un fichier de modèle 3D `.obj` et le convertir en une liste de triangles dans la scène. Attention aux performances sur les gros modèles.

### 5. Matériaux Avancés (Refactoring nécessaire)
* **Reflection (+0.5 pt)** : Effet miroir parfait. Demande d'ajouter un attribut `reflectivity` à l'interface `IPrimitive`, de recompiler tous les plugins, et d'ajouter le calcul du rayon réfléchi dans le `Renderer`.
* **Refraction (+1.0 pt)** : Déformation de la lumière à travers un objet (Loi de Snell-Descartes). Demande d'ajouter un attribut `ior` (Index of Refraction) à `IPrimitive` et de relancer des rayons déviés.
