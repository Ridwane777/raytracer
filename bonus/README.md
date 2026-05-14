# RayTracer — Moteur de rendu avec viewer SFML temps réel

Un raytracer écrit en C++ avec un mode rendu hors-ligne (image PPM/PNG) et un
**viewer SFML interactif** qui affiche le résultat dans une fenêtre et permet
de déplacer la caméra à la volée, avec relance progressive du rendu.

---

## Table des matières

1. [Architecture générale](#architecture-générale)
2. [Le viewer SFML](#le-viewer-sfml)
3. [Le système de threads](#le-système-de-threads)
4. [Le buffer de pixels](#le-buffer-de-pixels)
5. [La boucle principale](#la-boucle-principale)
6. [La caméra et les contrôles](#la-caméra-et-les-contrôles)
7. [Le pipeline de rendu](#le-pipeline-de-rendu)
8. [Pièges rencontrés et corrections](#pièges-rencontrés-et-corrections)
9. [Build & exécution](#build--exécution)

---

## Architecture générale

Le projet est organisé autour d'une séparation claire entre **données de
scène**, **moteur de rendu** et **affichage**.

```
┌─────────────────────────────────────────────────────────────────┐
│                          main / CLI                             │
│       Charge un fichier .cfg → construit une Scene              │
└───────────────────────┬─────────────────────────────────────────┘
                        │
              ┌─────────┴─────────┐
              ▼                   ▼
       ┌────────────┐      ┌──────────────┐
       │  Renderer  │      │  SFMLViewer  │   (mode interactif)
       │  (PPM/PNG) │      │              │
       └─────┬──────┘      └──────┬───────┘
             │                    │
             └──────┬─────────────┘
                    ▼
            ┌───────────────┐
            │     Scene     │
            │  ┌─────────┐  │
            │  │ Camera  │  │
            │  ├─────────┤  │
            │  │ Lights  │  │
            │  ├─────────┤  │
            │  │ Shapes  │  │
            │  └─────────┘  │
            └───────────────┘
```

Le `Renderer` est **stateless** : il prend une `Scene` et calcule la couleur
d'un rayon. Cette propriété est essentielle car elle permet d'en instancier un
par thread sans aucune synchronisation.

---

## Le viewer SFML

Le viewer (`SFMLViewer`) est une couche au-dessus du `Renderer` qui :

- ouvre une fenêtre SFML,
- crée une `sf::Texture` et un `sf::Sprite` qui s'affichent à l'écran,
- distribue le rendu sur plusieurs threads,
- met à jour la texture à 60 FPS,
- écoute les inputs clavier/souris pour déplacer la caméra,
- relance automatiquement le rendu à chaque mouvement.

### Pipeline d'affichage

```
   Renderer (N threads)                 Main thread (60 FPS)
   ──────────────────                  ──────────────────────
        │                                       │
        ▼                                       ▼
 ┌─────────────┐    écriture          ┌──────────────────┐
 │ RenderBuffer│ ────────────────────▶│  uploadTexture() │
 │  (uint8[])  │     (par tuiles)     └────────┬─────────┘
 └─────────────┘                               │
                                               ▼
                                       ┌────────────────┐
                                       │  sf::Texture   │
                                       └────────┬───────┘
                                                │
                                                ▼
                                       ┌────────────────┐
                                       │   sf::Sprite   │ → window.draw()
                                       └────────────────┘
```

La texture est créée à la résolution de rendu (par exemple 640×360), puis le
sprite est **mis à l'échelle** vers la taille de la fenêtre (par exemple
1280×720). Ça permet d'avoir un rendu rapide en basse résolution avec un
affichage plein écran net (interpolation bilinéaire).

### Configuration

```cpp
SFMLViewer::Config cfg;
cfg.windowWidth  = 1280;  // taille fenêtre
cfg.windowHeight = 720;
cfg.renderWidth  = 640;   // résolution réelle du raytracer
cfg.renderHeight = 360;
cfg.numThreads   = 0;     // 0 = auto (std::thread::hardware_concurrency())
cfg.maxDepth     = 5;     // profondeur max des rayons réfléchis/réfractés
cfg.showHUD      = true;
```

---

## Le système de threads

Le rendu est parallélisé via un **thread pool maison** qui distribue des
**tuiles** (bandes horizontales de pixels) aux workers.

### Pourquoi un pool et pas `std::async` ?

- Création de threads coûteuse → on les recycle.
- On peut **annuler** un rendu en cours (utile quand la caméra bouge en
  continu).
- On peut **attendre** la fin de tous les workers (`waitAll()`) pour garantir
  la cohérence du buffer.

### Structure du pool

```
   submitTiles(tiles, task)
            │
            ▼
   ┌────────────────────┐
   │   queue<Job>       │  ← les tuiles à rendre
   └────────┬───────────┘
            │
            ▼
   ┌────────────────────┐
   │  condition_variable│
   └────────┬───────────┘
            │
   ┌────────┴────────┬────────┬────────┐
   ▼                 ▼        ▼        ▼
 worker 0       worker 1   worker 2   ...   worker N-1
   │                                          │
   └──── chacun pop un job, l'exécute, ───────┘
              décrémente _pending,
              notifie _doneCv quand 0
```

### Cycle de vie d'un batch

1. `submitTiles()` appelle d'abord `waitAll()` pour s'assurer qu'**aucun
   worker n'est encore actif** sur le batch précédent.
2. La file est remplie avec les nouvelles tuiles.
3. `_pending` est mis à `tiles.size()`.
4. `notify_all()` réveille tous les workers.
5. Chaque worker prend une tuile, l'exécute, décrémente `_pending`.
6. Quand `_pending == 0`, `_doneCv` est notifié → `waitAll()` rend la main.

### Annulation coopérative

Quand l'utilisateur bouge la caméra pendant un rendu :

```cpp
_cancelRender = true;     // drapeau atomique
_pool->waitAll();         // les workers vérifient le drapeau et sortent vite
_cancelRender = false;
// ... soumet un nouveau batch avec la nouvelle position caméra
```

Dans la boucle pixel, chaque worker teste `cancel.load()` à chaque ligne, ce
qui permet une interruption en quelques microsecondes.

### Découpage en tuiles

Une "tuile" actuelle est une **bande horizontale** de 32 pixels de haut sur
toute la largeur :

```cpp
std::vector<Tile> buildTiles(int tileHeight = 32);
// Pour une résolution 640×360, on a ⌈360/32⌉ = 12 tuiles.
```

**Pourquoi des bandes et pas des carrés ?** Plus simple. Si un jour on veut
améliorer la localité de cache ou l'équilibrage de charge, on peut passer à
des tuiles 2D (par exemple 64×64).

---

## Le buffer de pixels

Le `RenderBuffer` est le tampon partagé entre les threads de rendu et le
thread principal.

### Format

- **RGBA 8 bits par canal** (`std::vector<sf::Uint8>` de taille `width*height*4`).
- C'est le format attendu par `sf::Texture::update()`. Tout autre format
  (RGB 3 octets, float, etc.) produit des artefacts horizontaux (décalage de
  pitch).

### Thread-safety

- Chaque tuile écrit dans une **plage de lignes disjointe** (`y ∈ [startY,
  endY)`), donc deux workers n'écrivent jamais dans le même pixel.
- Les écritures octet par octet sont atomiques sur x86/ARM modernes, donc pas
  besoin de mutex par pixel.
- Le mutex (`_mutex`) ne protège que `uploadToTexture()`, et plus par
  prudence que par nécessité.

### Initialisation

```cpp
RenderBuffer::RenderBuffer(int w, int h)
    : _width(w), _height(h), _pixels(w * h * 4, 0)
{
    // Alpha à 255 partout, sinon les zones non encore rendues
    // sont transparentes et laissent voir le clear() de la fenêtre.
    for (size_t i = 3; i < _pixels.size(); i += 4)
        _pixels[i] = 255;
}
```

---

## La boucle principale

```cpp
void SFMLViewer::run() {
    launchRender();                    // premier rendu

    sf::Clock clock;
    while (_window.isOpen()) {
        float dt = clock.restart().asSeconds();

        handleEvents();                // pollEvent + clavier
        handleInput(dt);               // applique mouvements + relance rendu si bougé

        _buffer->uploadToTexture(_texture);  // copie buffer → GPU

        _window.clear(sf::Color(20, 20, 30));
        _window.draw(_sprite);
        if (_cfg.showHUD)      drawHUD();
        if (_cfg.showTileGrid) drawTileGrid();
        _window.display();             // limité à 60 FPS via setFramerateLimit
    }
}
```

Le thread principal **ne fait jamais** de raytracing. Il dessine
l'état courant du buffer, qui peut être partiellement rempli par les workers.
C'est ce qui donne le rendu progressif visible à l'écran.

---

## La caméra et les contrôles

### Modèle de caméra

La caméra a une position 3D et une rotation en **angles d'Euler** (yaw, pitch,
roll en degrés). À partir de ça on calcule trois axes orthogonaux :

```cpp
yaw   = rotation.Y * π/180
pitch = rotation.X * π/180

forward = (sin(yaw)·cos(pitch), -sin(pitch), cos(yaw)·cos(pitch))
right   = normalize(forward × worldUp)
up      = normalize(right × forward)
```

Ces vecteurs servent à la fois à :
- **calculer les rayons** lors du rendu (`getRayForPixel(x, y)`),
- **déplacer la caméra** dans le repère local (avancer = `position + forward * dt`).

### Mapping clavier

| Touche          | Action                       |
|-----------------|------------------------------|
| `Z` / `W` / `↑` | Avancer                      |
| `S` / `↓`       | Reculer                      |
| `Q` / `A`       | Strafe gauche                |
| `D`             | Strafe droite                |
| `R` / `Space`   | Monter                       |
| `F` / `LCtrl`   | Descendre                    |
| `←` / `→`       | Yaw (rotation horizontale)   |
| `PageUp` / `PgDn` | Pitch (rotation verticale) |
| Molette         | Avancer / reculer            |
| `+` / `-`       | Augmenter / diminuer vitesse |
| `H`             | Toggle HUD                   |
| `G`             | Toggle grille de tuiles      |
| `Esc`           | Quitter                      |

### Relance du rendu

Dans `handleInput()`, après avoir appliqué les déplacements :

```cpp
if (moved) {
    ++_renderCount;
    launchRender();
}
```

`launchRender()` se charge d'annuler le rendu en cours et d'en démarrer un
nouveau avec la nouvelle position caméra.

**Optimisation possible** : si le déplacement est continu (touche
maintenue), on lance un rendu par frame (60 Hz) alors qu'un rendu prend ~20
ms. Deux rendus sur trois sont annulés avant d'avoir produit quoi que ce
soit. Une version "throttle" qui ne relance qu'une fois le précédent terminé
peut être ajoutée via un drapeau `_cameraDirty` (voir code commenté dans
`SFMLViewer.cpp`).

---

## Le pipeline de rendu

### Pour chaque pixel

```
   (x, y) ──▶ Camera::getRayForPixel ──▶ Ray
                                          │
                                          ▼
                            Renderer::computeColor(ray, depth)
                                          │
                       ┌──────────────────┼──────────────────┐
                       ▼                  ▼                  ▼
                Trouve la plus    Si miroir/transparent  Sinon : shading
                proche intersect. → relance ray récursif  (lights + ambient)
                       │                  │                  │
                       └──────────────────┴──────────────────┘
                                          │
                                          ▼
                                       Color
                                          │
                                          ▼
                                    setPixel(x, y, ...)
```

### Calcul du rayon primaire

`Camera::getRayForPixel(x, y)` :

1. Normalise `(x, y)` en coordonnées NDC `[-1, +1]`.
2. Applique le ratio d'aspect et le FOV pour obtenir un point sur le plan
   image local.
3. Transforme ce vecteur du repère caméra vers le repère monde via les axes
   `forward`/`right`/`up`.
4. Construit un `Ray` avec origine = position caméra, direction = vecteur
   normalisé.

### Intersection

La scène contient une liste de **formes** (sphères, plans, etc.). Pour chaque
rayon, on teste l'intersection avec toutes les formes et on garde la plus
proche (`t` minimal positif).

> **Note** : sur des scènes plus grosses, on remplacerait cette boucle par une
> structure d'accélération (BVH, KD-tree). Pour quelques sphères, la boucle
> brute est largement suffisante.

### Shading (modèle de Phong)

À chaque point d'intersection :

```
color = ambient                                       (couleur indépendante de la lumière)
      + Σ_lights (diffuse · max(0, N·L) + specular)   (somme sur chaque lumière visible)
```

avec :
- `N` = normale à la surface,
- `L` = direction vers la lumière,
- `ambient`, `diffuse`, `specular` = propriétés du matériau.

Pour chaque lumière, on tire un **shadow ray** vers la source : s'il intersecte
une autre forme avant d'atteindre la lumière, le point est dans l'ombre et
cette lumière ne contribue pas. C'est ce qui produit les ombres portées
visibles à l'écran.

### Réflexion / réfraction (récursion)

Si le matériau est miroir ou transparent, on lance un nouveau rayon depuis le
point d'intersection (rayon réfléchi ou réfracté via Snell-Descartes) et on
mélange sa couleur avec la couleur de surface. La profondeur de récursion
est limitée par `maxDepth` pour éviter les boucles infinies (par exemple deux
miroirs face à face).

### Conversion vers SFML

Le `Renderer` travaille en `double` dans `[0, 1]`. Avant d'écrire dans le
buffer :

```cpp
static sf::Color toSFML(const Color& c) {
    Color clamped = c.clamp();             // [0, 1]
    return sf::Color(
        (sf::Uint8)(clamped.r * 255),
        (sf::Uint8)(clamped.g * 255),
        (sf::Uint8)(clamped.b * 255),
        255
    );
}
```

---

## Pièges rencontrés et corrections

Cette section documente les bugs rencontrés pendant le développement du
viewer. Garder cette trace évite de les reproduire.

### 1. Bandes horizontales décalées

**Symptôme** : l'image affichée était composée de fines bandes bleues et
noires, sans contenu visible.

**Cause** : un `swap()` était appelé **après chaque tuile** par chaque worker
dans un double-buffer. Comme N workers swapaient en parallèle, le buffer
"front" et le buffer "back" étaient en permanence dans un état incohérent —
les tuiles écrasaient celles des autres workers.

**Correction** : suppression complète du double-buffer. Un seul buffer,
chaque tuile écrit dans sa plage de lignes (disjointe), pas de swap.

### 2. Image transparente

**Symptôme** : la fenêtre affichait uniformément la couleur du `clear()`
(bleu nuit `(20, 20, 30)`).

**Cause** : le buffer était initialisé à zéro → alpha = 0 → sprite
transparent → on voyait le clear de la fenêtre derrière.

**Correction** : initialiser le canal alpha à 255 dans le constructeur du
`RenderBuffer`.

### 3. Bandes lors d'un mouvement de caméra

**Symptôme** : en maintenant une flèche, l'image se découpait en bandes
décalées les unes par rapport aux autres.

**Cause** : `handleInput()` est appelé à 60 Hz. Si la touche est maintenue,
`launchRender()` est appelé chaque frame. La version initiale de
`ThreadPool::submitTiles()` vidait la file des **jobs en attente** mais
n'attendait pas les workers **en cours d'exécution**. Résultat : les workers
de l'ancien batch continuaient d'écrire dans le buffer pendant que ceux du
nouveau batch démarraient, mélangeant deux positions caméra.

**Correction** : `submitTiles()` appelle d'abord `waitAll()` pour garantir
qu'aucun worker n'est actif avant de soumettre le nouveau batch. Le drapeau
`_cancelRender` permet aux workers en vol de sortir vite.

### 4. Test critique : remplir le buffer de rouge

Quand l'image ne s'affichait pas, le test décisif a été de **forcer le buffer
à du rouge pur** dans le constructeur :

```cpp
for (size_t i = 0; i < _pixels.size(); i += 4) {
    _pixels[i+0] = 255; _pixels[i+1] = 0;
    _pixels[i+2] = 0;   _pixels[i+3] = 255;
}
```

- Si la fenêtre devient rouge → la chaîne d'affichage marche, le bug est dans
  le raytracer ou la scène.
- Si elle reste de la couleur de clear → le bug est dans l'upload de la
  texture ou le sprite.

C'est l'approche **bisection** classique pour isoler un bug d'affichage en
quelques minutes.

---

## Build & exécution

### Dépendances

- C++17 ou plus récent
- SFML 2.5+
- Un système avec une police monospace (chargée depuis
  `/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf` sur Linux, fallback
  Mac et Windows)

### Compilation

```bash
make
```

### Lancement

```bash
# Mode interactif (viewer SFML)
./raytracer scenes/reproduce_reference.cfg --interactive

# Mode hors-ligne (image PPM)
./raytracer scenes/reproduce_reference.cfg > image.ppm
```

### Format de scène

Les scènes sont décrites dans des fichiers `.cfg` (libconfig). Exemple
minimal :

```
camera = {
    resolution = { width = 640; height = 360; };
    position   = { x = 0;  y = 5; z = 25; };
    rotation   = { x = -10; y = 3; z = 0; };
    fieldOfView = 72.0;
};

primitives = {
    spheres = (
        { x = -3; y = 0; z = 5;  r = 1; color = { r = 255; g = 0; b = 0; }; },
        { x =  0; y = 0; z = 5;  r = 2; color = { r = 0; g = 255; b = 0; }; }
    );
};

lights = {
    ambient  = 0.2;
    diffuse  = 0.8;
    point    = ( { x = 10; y = 10; z = 10; } );
};
```

---

## Pistes d'amélioration

- **Tuiles 2D** (carrés 64×64) au lieu de bandes horizontales, pour un
  remplissage visuellement plus discret et un meilleur équilibrage de charge.
- **Throttle des relances** : ne relancer le rendu qu'une fois le précédent
  terminé (flag `_cameraDirty`), au lieu d'annuler à chaque frame.
- **BVH** pour accélérer le tracé sur des scènes denses.
- **Anti-aliasing** (supersampling ou MSAA logiciel) : à 4 samples par pixel
  les bords des sphères deviennent nets.
- **Skybox / environment map** au lieu d'une couleur d'arrière-plan unie.
- **Save snapshot** : touche `P` pour exporter l'image courante en PNG.
