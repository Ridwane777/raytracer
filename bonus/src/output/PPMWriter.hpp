#ifndef PPMWRITER_HPP_
#define PPMWRITER_HPP_

#include "../math/Color.hpp"
#include <string>
#include <vector>

class PPMWriter {
public:
    
    static void write(const std::string& filepath,
                      const std::vector<std::vector<Color>>& pixels);
};

#endif 
