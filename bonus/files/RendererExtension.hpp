// RendererExtension.hpp
// À inclure dans Renderer.hpp pour exposer computeColor au SFMLViewer.
//
// Dans votre Renderer.hpp existant, rendez computeColor public
// ou ajoutez cet alias :
//
//   public:
//       Color computeColorPublic(const Ray& ray, int depth) const {
//           return computeColor(ray, depth);
//       }
//
// Alternativement, déclarez SFMLViewer friend de Renderer :
//
//   friend class RayTracer::SFMLViewer;
//
// Cette approche est recommandée si vous ne voulez pas toucher
// à la visibilité des méthodes du Renderer.

// ─────────────────────────────────────────────────────────────
// PATCH MINIMAL pour Renderer.hpp
// ─────────────────────────────────────────────────────────────
// Ajoutez dans la section public de Renderer :
//
//  Color computeColorPublic(const Ray& ray, int depth = 0) const {
//      return computeColor(ray, depth);
//  }
//
// ─────────────────────────────────────────────────────────────
// PATCH pour main.cpp
// ─────────────────────────────────────────────────────────────
//
// Option A : mode normal (PPM)
//   int main(int argc, char** argv) {
//       Scene scene = SceneParser{}.parse(argv[1]);
//       Renderer renderer(scene);
//       PPMWriter::write("image.ppm", renderer.render());
//   }
//
// Option B : mode SFML interactif
//   int main(int argc, char** argv) {
//       Scene scene = SceneParser{}.parse(argv[1]);
//
//       SFMLViewer::Config cfg;
//       cfg.renderWidth  = scene.camera.width;
//       cfg.renderHeight = scene.camera.height;
//       cfg.numThreads   = 0;   // auto
//       cfg.maxDepth     = 0;
//
//       SFMLViewer viewer(std::move(scene), cfg);
//       viewer.run();
//   }
//
// Option C : flag --sfml
//   int main(int argc, char** argv) {
//       bool sfmlMode = (argc >= 3 && std::string(argv[2]) == "--sfml");
//       Scene scene = SceneParser{}.parse(argv[1]);
//       if (sfmlMode) {
//           SFMLViewer::Config cfg;
//           cfg.renderWidth  = scene.camera.width;
//           cfg.renderHeight = scene.camera.height;
//           SFMLViewer(std::move(scene), cfg).run();
//       } else {
//           Renderer renderer(scene);
//           PPMWriter::write("image.ppm", renderer.render());
//       }
//   }
