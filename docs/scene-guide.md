# Writing a scene: a beginner's guide

This guide shows how to describe a 3D picture in a text file and render it. You do not need to know C++ or
3D math: a scene is a list of objects with a position, a size and a color, plus a camera and some lights.

- [1. Your first render in 5 minutes](#1-your-first-render-in-5-minutes)
- [2. How the 3D space works](#2-how-the-3d-space-works)
- [3. File structure](#3-file-structure)
- [4. The camera](#4-the-camera)
- [5. The background](#5-the-background)
- [6. Objects](#6-objects)
- [7. Materials: shiny, mirror, glass](#7-materials-shiny-mirror-glass)
- [8. Moving, turning and stretching objects](#8-moving-turning-and-stretching-objects)
- [9. Lights](#9-lights)
- [10. Command line options](#10-command-line-options)
- [11. Recipes](#11-recipes)
- [12. Troubleshooting](#12-troubleshooting)
- [Cheat sheet](#cheat-sheet)

## 1. Your first render in 5 minutes

Build the program once (see the [README](../README.md#build--usage)), then create a file `scenes/my_scene.cfg`:

```
camera:
{
    resolution = { width = 800; height = 450; };
    position = { x = 0.0; y = 5.0; z = 40.0; };
    rotation = { x = -5.0; y = 0.0; z = 0.0; };
    fieldOfView = 60.0;
};

background:
{
    top    = { r = 40;  g = 90;  b = 200; };
    bottom = { r = 220; g = 230; b = 255; };
};

primitives:
{
    spheres = (
        { x = 0.0; y = 5.0; z = 0.0; r = 5.0; color = { r = 230; g = 60; b = 60; }; shininess = 80.0; }
    );
    planes = (
        { axis = "Y"; position = 0.0; color = { r = 200; g = 200; b = 200; }; reflectivity = 0.3; }
    );
};

lights:
{
    ambient = 0.2;
    diffuse = 0.8;
    point = (
        { x = 20.0; y = 30.0; z = 30.0; color = { r = 255; g = 255; b = 255; }; }
    );
};
```

Render it and convert the result to PNG (any image viewer that reads `.ppm` also works):

```bash
./raytracer scenes/my_scene.cfg -o my_scene.ppm
convert my_scene.ppm my_scene.png      # ImageMagick, optional
```

You get a red ball sitting on a light gray, slightly reflective floor under a blue sky. Change a number, render
again, look: this is the whole workflow.

## 2. How the 3D space works

Every position is three numbers, `x`, `y` and `z`:

```
            y (up)
            |
            |
            +------ x (right)
           /
          /
         z (toward you)
```

- `x` goes from left (negative) to right (positive).
- `y` goes from down (negative) to up (positive). The floor is usually a plane at `y = 0` or a little below.
- `z` goes from far away (negative) to close to you (positive). The camera usually sits at a positive `z`
  (for example `z = 40`) and looks toward negative `z`.

There is no unit: `1.0` is whatever you decide (a centimeter, a meter). Only proportions matter: a sphere of
radius 5 seen from 40 units away looks the same as a sphere of radius 50 seen from 400 units away.

Colors are three numbers from 0 to 255: red, green, blue. `{ r = 255; g = 255; b = 255; }` is white,
`{ r = 0; g = 0; b = 0; }` is black, `{ r = 255; g = 200; b = 60; }` is gold. Any online "color picker" gives
these values.

Angles are in degrees.

## 3. File structure

A scene file has up to four blocks. Each one is optional, missing values get a default.

```
camera:      { ... };   # where we look from
background:  { ... };   # what is behind everything (default: black)
primitives:  { ... };   # the objects
lights:      { ... };   # the lights
```

Syntax rules (the format is [libconfig](https://hyperrealm.github.io/libconfig/)):

- Each setting ends with `;`, for example `r = 5.0;`.
- A group of settings goes between `{ }`, a list of groups between `( )` with commas between items.
- Lines starting with `#` are comments.
- Write decimals with a dot: `5.0`, not `5,0`. Whole numbers like `5` also work.
- Names are case sensitive: `fieldOfView`, not `fieldofview`.

If the file has a syntax error, the program tells you the line number.

## 4. The camera

```
camera:
{
    resolution = { width = 1920; height = 1080; };
    position = { x = 0.0; y = 10.0; z = 70.0; };
    rotation = { x = -10.0; y = 0.0; z = 0.0; };
    fieldOfView = 50.0;
    antialiasing = 4;
};
```

| Setting | Meaning | Default |
| --- | --- | --- |
| `resolution` | Image size in pixels | 1920 x 1080 |
| `position` | Where the camera stands | 0, 0, 0 |
| `rotation` | Where it looks, in degrees. Without rotation it looks toward negative `z`. `x` tilts up (positive) or down (negative), `y` turns left (positive) or right (negative), `z` rolls the picture | 0, 0, 0 |
| `fieldOfView` | Vertical zoom angle in degrees. Small (30) = telephoto, large (80) = wide angle | 72 |
| `antialiasing` | Samples per pixel. 1 = fast with jagged edges, 4 = smooth edges and 4 times slower | 1 |

Tip: when the camera is high above the scene (large `y`), tilt it down with a negative `rotation.x`.

## 5. The background

What a ray sees when it hits nothing. It also shows in mirrors and glass, so a background makes reflective
scenes much richer.

```
background:
{
    top    = { r = 20;  g = 24; b = 70; };    # color when looking up
    bottom = { r = 255; g = 120; b = 60; };   # color at the horizon and below
};
```

Or a single flat color:

```
background:
{
    color = { r = 10; g = 10; b = 20; };
};
```

Without a `background` block, the background is black.

## 6. Objects

All objects go inside `primitives`, grouped by kind. Each kind is a list:

```
primitives:
{
    spheres   = ( { ... }, { ... } );
    planes    = ( { ... } );
    cylinders = ( ... );
    cones     = ( ... );
    toruses   = ( ... );
    triangles = ( ... );
    objs      = ( ... );
};
```

Every object accepts a `color`, the material settings of [section 7](#7-materials-shiny-mirror-glass) and the
transformations of [section 8](#8-moving-turning-and-stretching-objects).

### Sphere

```
{ x = 0.0; y = 5.0; z = 0.0; r = 5.0; color = { r = 230; g = 60; b = 60; }; }
```

`x, y, z` is the center, `r` the radius.

### Plane

An infinite flat surface, perfect for floors and walls.

```
{ axis = "Y"; position = -2.0; color = { r = 200; g = 200; b = 200; }; }
```

`axis` is the direction the plane faces:

- `"Y"`: a horizontal floor or ceiling at height `position`.
- `"Z"`: a back wall at depth `position` (use a negative value to put it behind the objects).
- `"X"`: a side wall at `x = position`.

### Cylinder

A vertical tube, open at both ends (no lid).

```
{ x = -18.0; y = -10.0; z = 0.0; r = 6.0; height = 16.0; color = { r = 235; g = 230; b = 220; }; }
```

`x, y, z` is the center of the **bottom**, the tube rises by `height` along `y`. `r` is the radius.
Seen from above, you can look inside it: put a sphere or a model on top to hide the opening.

### Cone

A vertical cone, base at the bottom and tip at the top.

```
{ x = 0.0; y = 0.0; z = 0.0; height = 10.0; angle = 20.0; color = { r = 255; g = 80; b = 80; }; }
```

`x, y, z` is the center of the base, `height` the distance to the tip. `angle` (degrees) controls how wide it
is: 10 is a needle, 45 is a wide cone. The base radius equals `height * tan(angle)`; the `r` setting is ignored.

### Torus (ring, donut)

```
{ x = 0.0; y = 0.0; z = 0.0; r1 = 12.0; r2 = 1.8; color = { r = 255; g = 190; b = 60; }; }
```

`r1` is the radius of the ring (center to the middle of the tube), `r2` the thickness of the tube. Without
rotation the ring lies flat like a ring on a table. Use `rotation = { x = 90.0; ... }` to stand it up facing the
camera, or `rotation = { z = 90.0; ... }` to stand it up sideways.

### Triangle

```
{ a = { x = 0.0; y = 0.0; z = 0.0; };
  b = { x = 10.0; y = 0.0; z = 0.0; };
  c = { x = 5.0; y = 8.0; z = 0.0; };
  color = { r = 80; g = 200; b = 120; }; }
```

The three corners `a`, `b` and `c`. Useful for small custom shapes; for bigger ones use a model.

### 3D model (`.obj` file)

```
{ filepath = "models/diamond.obj"; x = 0.0; y = 10.0; z = 0.0;
  color = { r = 200; g = 230; b = 255; };
  scale = { x = 0.9; y = 0.9; z = 0.9; }; rotation = { x = 0.0; y = 25.0; z = 0.0; }; }
```

Loads a Wavefront `.obj` file (the most common 3D export format). Only the shape is used (`v` and `f` lines),
colors and textures in the file are ignored. `filepath` is relative to the folder you run the program from.
`x, y, z` moves the model's own origin. The `models/` folder contains `cube`, `diamond`, `crystal` and `rocket`.

Every face becomes a triangle, and there is no acceleration structure: a model with thousands of faces will
render slowly.

## 7. Materials: shiny, mirror, glass

These settings work on every object. All are optional.

| Setting | Range | Effect | Default |
| --- | --- | --- | --- |
| `color` | 0 to 255 each | Base color | black |
| `shininess` | 0 to about 300 | Size of the bright highlight. 0 = matte (chalk), 20 = soft plastic, 100+ = glossy, 300 = tiny sharp spot (glass, chrome) | 0 |
| `specular` | 0 to 255 each | Color of the highlight. White for plastic and glass, tinted (gold, copper) for metals | white |
| `reflectivity` | 0.0 to 1.0 | Mirror effect. 0 = none, 0.3 = polished floor, 0.9 = mirror or chrome | 0.0 |
| `transparency` | 0.0 to 1.0 | 0 = solid, 0.9 = glass. The rest of the light comes from what is seen through the object | 0.0 |
| `refractiveIndex` | 1.0 and above | How much a transparent object bends light. 1.0 = no bending, 1.33 = water, 1.5 = glass, 2.4 = diamond | 1.0 |

Ready-made materials:

```
# Red plastic
color = { r = 220; g = 40; b = 40; }; shininess = 60.0;

# Chrome
color = { r = 230; g = 230; b = 235; }; reflectivity = 0.9; shininess = 200.0;

# Gold
color = { r = 255; g = 190; b = 60; }; reflectivity = 0.5; shininess = 120.0;
specular = { r = 255; g = 220; b = 150; };

# Glass
color = { r = 255; g = 255; b = 255; }; transparency = 0.9; refractiveIndex = 1.5; shininess = 300.0;

# Polished dark floor (plane)
color = { r = 30; g = 30; b = 40; }; reflectivity = 0.5; shininess = 60.0;
```

Note: shadows are fully dark even behind glass (the renderer does not compute light passing through
transparent objects).

## 8. Moving, turning and stretching objects

Every object accepts three optional transformations:

```
rotation = { x = 90.0; y = 0.0; z = 0.0; };    # turn, in degrees, around each axis
scale    = { x = 2.0; y = 1.0; z = 1.0; };     # stretch: here twice as wide
shear    = { xy = 0.5; };                      # slant (xy, xz, yx, yz, zx, zy)
```

- Rotation and scale happen around the object's own position (`x, y, z`), so the object stays in place.
- A sphere with `scale = { x = 1.0; y = 0.5; z = 1.0; }` becomes a flattened pebble.
- A cylinder with `rotation = { z = 90.0; ... }` lies on its side.
- A plane with a rotation becomes a slope.

See [transformations.md](transformations.md) for the math.

## 9. Lights

```
lights:
{
    ambient = 0.1;
    diffuse = 0.9;
    point = (
        { x = 20.0; y = 40.0; z = 30.0; color = { r = 255; g = 240; b = 220; }; }
    );
    directional = (
        { x = -1.0; y = -1.0; z = -0.5; color = { r = 255; g = 255; b = 255; }; }
    );
};
```

| Setting | Meaning | Default |
| --- | --- | --- |
| `ambient` | Light everywhere, even in shadows. 0.05 = dramatic and dark, 0.4 = flat and bright | 0.4 |
| `diffuse` | Strength of every point and directional light | 0.6 |
| `point` | A light bulb at position `x, y, z`. It shines in all directions and casts shadows | none |
| `directional` | A light from very far away, like the sun. `x, y, z` is the **direction the light travels**: `y = -1` shines downward | none |

Each light has a `color`. Colored lights are the easiest way to get a mood: a magenta light on one side and a
cyan one on the other give a neon look; a warm orange light low on the horizon gives a sunset.

All lights add up: three white lights at full strength wash the image out. Use darker light colors
(for example `{ r = 120; g = 120; b = 120; }`) for fill lights, or lower `diffuse`.

The strength does not decrease with distance: a point light lights a far object as much as a close one.

## 10. Command line options

```
./raytracer <scene.cfg> [options]
```

| Option | Effect |
| --- | --- |
| `-o FILE`, `--output FILE` | Write the image to `FILE` instead of `output.ppm` |
| `--res WxH` | Override the scene resolution, for example `--res 640x360` for a fast preview |
| `--depth D` | How many times a ray may bounce between mirrors or through glass (default 4). Facing mirrors need 8 to 10 |
| `--sfml` | Open an interactive window instead of writing a file (only if the program was built with SFML 2) |
| `--threads N`, `--no-hud`, `--tile-debug` | Window mode only: render threads, hide the overlay, show render tiles |
| `-h`, `--help` | Show the help |

Work fast: preview at low resolution with `antialiasing = 1`, and only render the final image at full size
with `antialiasing = 4`.

```bash
./raytracer scenes/my_scene.cfg --res 480x270 -o preview.ppm
```

## 11. Recipes

The scenes below are in `scenes/showcase/`; open them as starting points.

| Effect | How | Example |
| --- | --- | --- |
| Sunset sky | `background` from dark blue (top) to orange (bottom), a warm `directional` light pointing toward the camera (`z` positive), a reflective floor | `sunset_lake.cfg` |
| Neon | Very dark background and floor, `ambient` around 0.05, two strongly colored point lights on each side, glossy objects | `neon_rings.cfg` |
| Infinite reflections | Two `"X"` planes facing each other with `reflectivity = 0.9`, render with `--depth 10` | `mirror_hall.cfg` |
| Museum spotlight | Objects on cylinder pedestals, one colored point light above each, a dark back wall | `gem_gallery.cfg` |
| Things that look heavy | Put objects exactly on the floor: a sphere of radius `r` on the floor `y = 0` has `y = r` | |
| Depth | Place objects at different `z`, not all on one line, and turn the camera a little (`rotation.y`) | |

## 12. Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| The image is all black | No light reaches the scene, or the camera looks the wrong way. Check `lights`, raise `ambient`, check the camera `rotation` |
| `Parse error ... at line N` | A missing `;`, an unclosed `{` or `(`, or a comma instead of a dot in a number |
| An object is missing | It is behind the camera (larger `z` than the camera), outside the field of view, or inside another object |
| Everything is white or washed out | Too many or too strong lights: darken light colors, lower `diffuse` or `ambient` |
| Mirrors look black in the middle | The ray ran out of bounces: raise `--depth` |
| `Cannot open .obj file` | `filepath` is relative to where you run the program: run it from the repository root |
| Render is very slow | Lower `antialiasing`, use `--res` for previews, avoid large `.obj` models |

## Cheat sheet

Every setting in one place. This is a syntax reference: the objects overlap, copy only the lines you need.

```
camera:
{
    resolution = { width = 1920; height = 1080; };
    position = { x = 0.0; y = 10.0; z = 70.0; };
    rotation = { x = -10.0; y = 0.0; z = 0.0; };
    fieldOfView = 50.0;
    antialiasing = 4;
};
background: { top = { r = 20; g = 24; b = 70; }; bottom = { r = 255; g = 120; b = 60; }; };
primitives:
{
    spheres   = ( { x = 0.0; y = 5.0; z = 0.0; r = 5.0; color = { r = 230; g = 60; b = 60; }; } );
    planes    = ( { axis = "Y"; position = 0.0; color = { r = 200; g = 200; b = 200; }; } );
    cylinders = ( { x = 0.0; y = 0.0; z = 0.0; r = 2.0; height = 10.0; color = { r = 200; g = 200; b = 200; }; } );
    cones     = ( { x = 0.0; y = 0.0; z = 0.0; height = 10.0; angle = 20.0; color = { r = 200; g = 200; b = 200; }; } );
    toruses   = ( { x = 0.0; y = 0.0; z = 0.0; r1 = 10.0; r2 = 2.0; color = { r = 200; g = 200; b = 200; }; } );
    triangles = ( { a = { x = 0.0; y = 0.0; z = 0.0; }; b = { x = 5.0; y = 0.0; z = 0.0; }; c = { x = 0.0; y = 5.0; z = 0.0; }; } );
    objs      = ( { filepath = "models/cube.obj"; x = 0.0; y = 0.0; z = 0.0; } );
};
lights:
{
    ambient = 0.1;
    diffuse = 0.9;
    point       = ( { x = 20.0; y = 40.0; z = 30.0; color = { r = 255; g = 255; b = 255; }; } );
    directional = ( { x = -1.0; y = -1.0; z = -0.5; color = { r = 255; g = 255; b = 255; }; } );
};

material settings (any object): color, shininess, specular, reflectivity, transparency, refractiveIndex
transformations (any object): rotation, scale, shear
```
