# Explications des correctifs : Mathématiques, Caméra et Transform

Ce document détaille les corrections mathématiques et logiques qui ont été apportées aux classes de base du projet Raytracer, en particulier sur le comportement de la caméra et la gestion des normales lors des transformations géométriques.

---

## 1. La rotation de la Caméra (`Camera::getScreen`)

### Le problème initial
Dans la première implémentation, la méthode `Camera::getScreen()` générait le plan de l'écran en hardcodant l'axe de vue vers `-Z`. L'attribut `rotation` de la caméra n'était jamais utilisé. Par conséquent, changer la rotation de la caméra ne modifiait pas la direction des rayons générés : la caméra regardait toujours dans la même direction.

### La solution
Pour qu'une caméra puisse regarder dans n'importe quelle direction, le plan de l'écran (qui représente les pixels que l'on va projeter) doit tourner *avec* la caméra.

Nous avons appliqué le processus suivant :
1. **Création d'une matrice de rotation globale** : On multiplie les matrices de rotation X, Y et Z en fonction des angles définis dans `camera.rotation`.
2. **Définition des vecteurs de l'écran dans l'espace local** : On définit le point d'origine du plan, le vecteur du bas (`bottom_side`) et le vecteur de gauche (`left_side`) comme si la caméra était à l'origine `(0, 0, 0)` et regardait vers `-Z`.
3. **Application de la rotation** : On multiplie ces trois vecteurs locaux par la matrice de rotation.
4. **Translation finale** : On ajoute la `position` de la caméra au point d'origine tourné pour le placer correctement dans le monde.

Ainsi, lorsque la caméra tourne, le plan de l'écran pivote exactement de la même manière, et les rayons générés par `getRayForPixel()` partent dans la bonne direction.

---

## 2. Le Scale, les Normales et la méthode `transpose()`

C'est l'un des pièges classiques en rendu 3D. 

### Pourquoi les vecteurs normaux sont-ils spéciaux ?
Un vecteur normal indique la direction perpendiculaire à une surface. Contrairement aux points ou aux vecteurs de direction (comme un rayon de lumière), **les normales ne réagissent pas de la même manière aux transformations non uniformes (Scale asymétrique)**.

Imaginez un cercle parfait avec une normale pointant vers le haut à 45°. Si on étire ce cercle uniquement sur l'axe X (Scale X = 2.0), le cercle devient une ellipse. La surface s'aplatit.
* Si on applique la même matrice de Scale à notre normale, elle va s'allonger sur l'axe X et **va "pencher" du mauvais côté**. Elle ne sera plus perpendiculaire à la nouvelle surface de l'ellipse !

### La règle mathématique
En mathématiques, les vecteurs normaux sont considérés comme des "covecteurs". Pour qu'un vecteur normal reste orthogonal à sa surface après une transformation $M$, il ne faut pas le multiplier par $M$, mais par **la transposée de l'inverse de $M$** : $(M^{-1})^T$.

### L'implémentation dans `Transform::getNormal()`
Dans le design pattern Decorator (`Transform`), l'objet possède deux matrices :
* `_localToWorld` : La matrice de transformation de base $M$.
* `_worldToLocal` : L'inverse de cette matrice $M^{-1}$.

Pour transformer un point ou un vecteur directionnel de l'espace local vers l'espace monde, on utilise `_localToWorld`. 
Mais pour **transformer une normale** de l'espace local vers l'espace monde, la formule mathématique nous dit d'utiliser $(M^{-1})^T$. 
Puisque nous avons déjà calculé $M^{-1}$ (qui est `_worldToLocal`), il nous suffit d'utiliser **sa transposée**.

C'est pour cela que la méthode `Matrix4x4::transpose()` a été ajoutée, et que `Transform::getNormal()` s'écrit désormais ainsi :

```cpp
Vector3D Transform::getNormal(const Point3D& point) const {
    // 1. On ramène le point dans l'espace local
    Point3D localPoint = _worldToLocal.transformPoint(point);
    
    // 2. On récupère la normale de la primitive de base (ex: la sphère parfaite)
    Vector3D localNormal = _wrapped->getNormal(localPoint);
    
    // 3. On ramène la normale dans le monde en utilisant la Transposée de l'Inverse
    return _worldToLocal.transpose().transformVector(localNormal).normalize();
}
```

Grâce à cette correction, si une primitive subit un Scale (ex: transformer une sphère en ovale), les calculs de lumière (qui dépendent des normales) resteront physiquement justes et n'auront pas d'ombres déformées.

---

## 3. Autres corrections mineures
* **`Matrix4x4::transformRay`** : Un bug critique transformait un rayon non initialisé (`Ray r;`) au lieu du rayon passé en paramètre. Il a été corrigé pour utiliser le paramètre `ray` et normaliser sa direction.
* **Ordre d'initialisation** : Résolution de warnings stricts (`-Werror`) dans le constructeur de `Transform`.
* **Inclusions** : Correction des chemins relatifs vers les fichiers mathématiques dans les interfaces (`IPrimitive.hpp`, `HitInfo.hpp`).
