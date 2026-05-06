#ifndef PPMWRITER_HPP_
#define PPMWRITER_HPP_

#include "../math/Color.hpp"
#include <string>
#include <vector>

class PPMWriter {
public:
    // Format PPM P3 (ASCII).
    // En-tête : "P3\nwidth height\n255\n"
    // Pixels : "r g b\n" par pixel, ligne par ligne (de haut en bas).
    static void write(const std::string& filepath,
                      const std::vector<std::vector<Color>>& pixels);
};

#endif /* !PPMWRITER_HPP_ */
