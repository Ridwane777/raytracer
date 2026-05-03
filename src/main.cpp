#include "src/math/Rectangle3D.hpp"
#include "src/math/Color.hpp"
#include "src/math/Matrix4x4.hpp"
#include "Error.hpp"

static void usage() {
    std::cout << "USAGE: ./raytracer <SCENE_FILE>\n"
    << "   SCENE_FILE: scene configuration" << std::endl;
}

int main(int ac, char **av)
{
    try {
        if (ac != 2) {
            throw Error("Invalid number of arguments.");
        }

        std::string arg = av[1]; 
        if (arg == "--help") {
            usage();
            return 0;
        } else if (arg.find(".cfg") != std::string::npos) {
            // Traiter la scène ici
            return 0;
        } else {
            throw Error("Unknown argument or file format.");
        }

    } catch(const std::exception &e) {
        std::cout << e.what() << std::endl;
        return 84;
    };
    return 0;
}