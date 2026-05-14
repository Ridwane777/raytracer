#ifndef COLOR_HPP_
#define COLOR_HPP_
#include <algorithm>

class Color {
public:
    double r;
    double g;
    double b; 

    Color();
    Color(double r, double g, double b);
    ~Color() = default;

    Color operator+(const Color& other) const;
    Color operator*(double factor) const;
    Color operator*(const Color& other) const; 
    Color clamp() const;                       

    int toInt_r() const; 
    int toInt_g() const;
    int toInt_b() const;
};

#endif 
