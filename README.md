# RayTracer

[![Build](https://github.com/Ridwane777/raytracer/actions/workflows/build.yml/badge.svg)](https://github.com/Ridwane777/raytracer/actions/workflows/build.yml)

A C++20 ray tracer that renders scene files to `.ppm` images, with primitives and lights loaded as dynamic `.so` plugins.
Scenes are plain text files: the [scene guide](docs/scene-guide.md) teaches how to write one, no 3D background needed.

<table>
  <tr>
    <td><img src="screenshots/sunset_lake.png" alt="Sunset lake" width="450"></td>
    <td><img src="screenshots/neon_rings.png" alt="Neon rings" width="450"></td>
  </tr>
  <tr>
    <td><img src="screenshots/mirror_hall.png" alt="Mirror hall" width="450"></td>
    <td><img src="screenshots/gem_gallery.png" alt="Gem gallery" width="450"></td>
  </tr>
</table>

<table>
  <tr>
    <td><img src="screenshots/08_showcase.png" alt="Showcase" width="300"></td>
    <td><img src="screenshots/01_cornell_box.png" alt="Cornell box" width="300"></td>
    <td><img src="screenshots/03_crystal_ball.png" alt="Crystal ball" width="300"></td>
  </tr>
  <tr>
    <td><img src="screenshots/07_solar_system.png" alt="Solar system" width="300"></td>
    <td><img src="screenshots/03_torus_knot.png" alt="Torus knot" width="300"></td>
    <td><img src="screenshots/06_temple.png" alt="Temple" width="300"></td>
  </tr>
</table>

## Architecture & design patterns

The engine follows SOLID principles and relies on four design patterns:

- **Factory** (`PrimitiveFactory`, `LightFactory`): loads `.so` plugins at runtime. The core has no knowledge of which primitives or lights exist.
- **Decorator** (`Transform`): handles spatial transformations. Each primitive is created at the origin (0,0,0) and wrapped in a `Transform` that applies a 4x4 transformation matrix.
- **Composite** (`PrimitiveList`): treats a collection of `IPrimitive` objects as a single one, so a ray is cast against the whole scene with one `hits()` call.
- **Builder** (`SceneBuilder`): builds the final `Scene` step by step from the data extracted by `SceneParser`.

## Features

### Primitives
- Sphere, plane, cylinder, cone (dynamic plugins)
- Triangle (`triangle.so`), using plane-polygon intersection
- Torus (`torus.so`), solved analytically as a quartic equation
- Wavefront `.obj` models, triangulated on the fly

### Transformations
- Translation, rotation (`rotationX`, `Y`, `Z`), scale and shear
- Combined and inverted 4x4 matrices, applicable to any primitive without changing its code

### Lighting
- Ambient, directional and point lights (no distance falloff)
- Unlimited number of lights, contributions summed
- Colored (RGB) lights: final color is the product of object and light colors
- Drop shadows via shadow rays
- Phong reflection model (ambient, diffuse, specular)

### Materials
- Flat RGB color
- Transparency (alpha blending)
- Refraction (Snell's law, refractive indices, total internal reflection)
- Reflection (mirror), with bounce depth limited by `_maxDepth`

### Rendering
- Antialiasing by supersampling with random sub-pixel offsets
- Recursive rendering engine

### Background
- Sky gradient (`top` and `bottom` colors) or flat color, also seen in reflections and refractions

### Display & interface
- Scenes configured through text files (libconfig)
- Output as `.ppm` (P3) images, path set with `-o`
- Optional interactive SFML 2 viewer (`--sfml`)
- Errors reported on `stderr`, `--help` usage

## Build & usage

### Requirements
- A `C++20` compiler
- `cmake` (>= 3.17)
- `libconfig++` (`sudo dnf install libconfig-c++-devel` or `sudo apt install libconfig++-dev`)
- Optional: SFML 2.5+ (`libsfml-dev`) for the interactive viewer. Without it (or with SFML 3 only), the program
  builds without `--sfml`

### Build

```bash
cmake -B build -S .
cmake --build build
```

### Run

```bash
# Render the test scene to output.ppm
./raytracer scenes/simple_scene.cfg

# Fast preview, then the final image with more mirror bounces
./raytracer scenes/showcase/mirror_hall.cfg --res 640x360 -o preview.ppm
./raytracer scenes/showcase/mirror_hall.cfg --depth 10 -o mirror_hall.ppm
convert mirror_hall.ppm mirror_hall.png   # optional, ImageMagick
```

Options: `-o FILE`, `--res WxH`, `--depth D`, `--sfml` (see `--help`).

## Scene files

Scenes are libconfig text files describing a camera, a background, objects and lights. Start with the
**[scene guide](docs/scene-guide.md)**: a first render in 5 minutes, every object and material explained, recipes
and troubleshooting. Examples are in `scenes/`, the four images above in `scenes/showcase/`.

## Documentation

- [Scene guide](docs/scene-guide.md)
- [Architecture](docs/architecture.md)
- [Transformations](docs/transformations.md)
- [Math formulas](docs/formulas.md)
- [Camera and math fixes](docs/camera-math-fixes.md)
- [Bug fixes](docs/bug-fixes.md)

## Author

Ridwane777

## License

[MIT](LICENSE)
