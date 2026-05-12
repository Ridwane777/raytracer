# Formules Mathématiques du Raytracer

Ce document centralise toutes les formules mathématiques utilisées actuellement dans l'architecture de base, ainsi que celles qui seront nécessaires pour implémenter les prochaines fonctionnalités (Primitives, Lumières, Ombres, etc.).

---

## 1. Formules déjà implémentées (Le Socle)

### A. Vecteurs et Géométrie dans l'espace (`Vector3D`)
* **Norme (Longueur du vecteur)** : 
  `||V|| = sqrt(x² + y² + z²)`
* **Normalisation (Rendre un vecteur de longueur 1)** : 
  `V_norm = V / ||V||`
* **Produit Scalaire (Dot Product)** : Mesure à quel point deux vecteurs pointent dans la même direction. Si le résultat est 0, ils sont perpendiculaires.
  `A . B = (A.x * B.x) + (A.y * B.y) + (A.z * B.z)`
* **Produit Vectoriel (Cross Product)** : Trouve un vecteur perpendiculaire à deux autres vecteurs.
  `A x B = (A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y*B.x)`

### B. Le Rayon de Lumière (`Ray`)
* **Équation paramétrique d'un rayon** : Un point `P` sur le rayon à une distance `t`.
  `P(t) = Origine + t * Direction`

### C. La Caméra et le Plan Écran (`Camera`)
* **Taille du plan écran selon le FOV (Field of View)** :
  `demi_hauteur = tan((FOV * PI / 180) / 2)`
  `demi_largeur = demi_hauteur * (largeur_ecran / hauteur_ecran)`
* **Génération de rayon pour le pixel (u, v)** où u et v vont de 0 à 1 :
  `PointCible = OrigineEcran + u * VecteurBas + v * VecteurGauche`
  `DirectionRayon = normalize(PointCible - PositionCamera)`

### D. Matrices et Transformations (`Matrix4x4`, `Transform`)
* **Transformation d'un point / vecteur** : Multiplication Matrice 4x4 par Vecteur 4x1.
* **Transformation d'une Normale** (pour conserver la perpendicularité en cas de *Scale* non-uniforme) :
  `Normale_Monde = Transpose(Inverse(MatriceTransformation)) * Normale_Locale`

---

## 2. Formules des Fonctionnalités Avancées

### A. Intersections Géométriques (Primitives)
Le but de l'intersection est de trouver le plus petit `t > 0` tel que `Rayon(t) = Surface de l'objet`.

**La Sphère :**
Résolution d'une équation du second degré `at² + bt + c = 0` :
* `L = OrigineRayon - CentreSphere`
* `a = DirectionRayon . DirectionRayon` (Vaut 1 si la direction est normalisée)
* `b = 2 * (DirectionRayon . L)`
* `c = (L . L) - RayonSphere²`
* **Discriminant (Delta)** : `Δ = b² - 4ac`
  * Si `Δ < 0` : Pas d'intersection.
  * Si `Δ >= 0` : `t1 = (-b - sqrt(Δ)) / 2a` et `t2 = (-b + sqrt(Δ)) / 2a`. On garde le plus petit `t > 0`.

**Le Plan :**
Si le plan est défini par une normale `N` et passe par un point `A` :
* `t = ((A - OrigineRayon) . N) / (DirectionRayon . N)`
* Si `(DirectionRayon . N) == 0` : Le rayon est parallèle au plan (pas d'intersection).

**Le Cylindre et le Cône (Détails) :**
Ils se résolvent également avec une équation quadratique (`at² + bt + c = 0`) :
* **Cylindre** (axe Y) :
  * `a = Direction.x² + Direction.z²`
  * `b = 2 * (Direction.x * (Origine.x - Centre.x) + Direction.z * (Origine.z - Centre.z))`
  * `c = (Origine.x - Centre.x)² + (Origine.z - Centre.z)² - Rayon²`
* **Cône** (axe Y, sommet S) :
  * `k = tan(Angle)²`
  * `a = Direction.x² + Direction.z² - k * Direction.y²`
  * `b = 2 * (Direction.x*(Orig.x - S.x) + Direction.z*(Orig.z - S.z) - k*Direction.y*(Orig.y - S.y))`
  * `c = (Orig.x - S.x)² + (Orig.z - S.z)² - k*(Orig.y - S.y)²`

---

### B. Éclairage (Lumières)

Pour chaque lumière, on ajoute sa contribution à la couleur du pixel.

**Lumière Ambiante (Implémenté) :**
Représente la lumière résiduelle qui atteint tous les points uniformément.
* `Couleur_Finale = Intensité_Ambiante * Couleur_Objet`

**Lumière Diffuse (Loi de Lambert) (Implémenté) :**
Détermine la luminosité en fonction de l'angle entre la lumière et la normale.
* `Intensité_Diffuse = Max(0, Normale . DirectionLumière)`
* `Couleur = CouleurObjet * CouleurLumière * Intensité_Diffuse`
*(Note : La direction de la lumière doit pointer **vers** la source lumineuse pour le produit scalaire).*

**Lumière Spéculaire (Reflet brillant - Modèle de Phong) (Could) :**
Détermine le reflet du soleil sur une pomme brillante.
* **Vecteur de réflexion R** : `R = 2 * (Normale . DirectionLumière) * Normale - DirectionLumière`
* `Intensité_Spéculaire = Max(0, Vue . R) ^ Brillance` (où Vue est la direction vers la caméra, et Brillance est une constante comme 32 ou 64).

**Ombres portées (Drop Shadows) (Implémenté) :**
Avant d'ajouter la couleur d'une lumière, on lance un "Rayon d'Ombre" (`Shadow Ray`) depuis le point d'intersection vers la source lumineuse.
* Si `PrimitiveList->hits(ShadowRay)` renvoie un résultat, alors un objet bloque la lumière -> La contribution de cette lumière est 0 (Noir).

---

### C. Reflets et Réfraction (Implémenté - Moteur Récursif)

**Réflexion (Miroir) :**
Le rayon rebondit sur l'objet et repart dans la scène.
* **Nouvelle Direction** : `R = DirectionRayon - 2 * (DirectionRayon . Normale) * Normale`
* On rappelle `Renderer::computeColor()` avec ce nouveau rayon.

**Réfraction (Verre / Eau - Loi de Snell-Descartes) :**
La lumière traverse l'objet et change de direction selon un indice de réfraction `n` (ex: air = 1.0, verre = 1.5).
* Rapport des indices : `r = n1 / n2`
* `cosI = -(Normale . DirectionRayon)`
* `sinT² = r² * (1 - cosI²)`
* Si `sinT² > 1` : Réflexion totale interne (ça devient un miroir).
* Sinon, **Nouvelle Direction** : `T = r * DirectionRayon + (r * cosI - sqrt(1 - sinT²)) * Normale`
* **Mélange Final (Alpha Blending)** : `Couleur = (CouleurBase * (1 - reflectivite)) + (CouleurReflechie * reflectivite)`

---

### D. Triangles et Modèles 3D (À venir)

**Algorithme de Möller-Trumbore (Intersection Rayon-Triangle) :**
Permet de calculer très rapidement l'intersection sans pré-calculer le plan du triangle.
Pour un triangle défini par ses trois sommets `A`, `B`, `C` :
1. **Arêtes** : `E1 = B - A`, `E2 = C - A`
2. **Vecteur P** : `P = DirectionRayon x E2`
3. **Déterminant** : `det = E1 . P`
   * Si `-EPSILON < det < EPSILON`, le rayon est parallèle au triangle.
4. **Vecteur T** : `T = OrigineRayon - A`
5. **Coordonnée U barycentrique** : `u = (T . P) / det`. Si `u < 0` ou `u > 1`, le rayon rate le triangle.
6. **Vecteur Q** : `Q = T x E1`
7. **Coordonnée V barycentrique** : `v = (DirectionRayon . Q) / det`. Si `v < 0` ou `u + v > 1`, le rayon rate le triangle.
8. **Distance t** : `t = (E2 . Q) / det`. Si `t > EPSILON`, il y a intersection !
