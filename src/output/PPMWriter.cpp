#include "PPMWriter.hpp"
#include "../Error.hpp"
#include <fstream>

void PPMWriter::write(const std::string& filepath,
                      const std::vector<std::vector<Color>>& pixels)
{
    if (pixels.empty())
        throw Error("PPMWriter: pixel buffer is empty.");

    const int height = static_cast<int>(pixels.size());
    const int width  = static_cast<int>(pixels[0].size());

    std::ofstream file(filepath);
    if (!file.is_open())
        throw Error("PPMWriter: cannot open file: " + filepath);

    // En-tête PPM P3
    file << "P3\n"
         << width << " " << height << "\n"
         << "255\n";

    // Pixels ligne par ligne, de haut en bas
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Color c = pixels[y][x].clamp();
            file << c.toInt_r() << " "
                 << c.toInt_g() << " "
                 << c.toInt_b();
            if (x < width - 1)
                file << "  ";
        }
        file << "\n";
    }
}
