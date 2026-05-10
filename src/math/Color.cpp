#include "Color.hpp"

Color::Color() : r(0), g(0), b(0)
{
}

Color::Color(double r, double g, double b) : r(r), g(g), b(b) {}

Color Color::operator*(double factor) const {
    return Color(r * factor, g * factor, b * factor);
}

Color Color::operator+(const Color& other) const {
    return Color(r + other.r, g + other.g, b + other.b);
}

Color Color::operator*(const Color& other) const {
    return Color(r * other.r, g * other.g, b * other.b);
}

Color Color::clamp() const {
    return Color(
        std::clamp(r, 0.0, 1.0),
        std::clamp(g, 0.0, 1.0),
        std::clamp(b, 0.0, 1.0)
    );
}

int Color::toInt_r() const {
    return static_cast<int>(255.99 * std::clamp(r, 0.0, 1.0));
}

int Color::toInt_g() const {
    return static_cast<int>(255.99 * std::clamp(g, 0.0, 1.0));
}

int Color::toInt_b() const {
    return static_cast<int>(255.99 * std::clamp(b, 0.0, 1.0));
}
