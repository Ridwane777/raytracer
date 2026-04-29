#ifndef COLOR_HPP_
#define COLOR_HPP_

class Color {
public:
    double r;
    double g;
    double b; // valeurs dans [0.0, 1.0]

    Color();
    Color(double r, double g, double b);
    ~Color() = default;

    Color operator+(const Color& other) const;
    Color operator*(double factor) const;
    Color operator*(const Color& other) const; // composante par composante
    Color clamp() const;                       // force dans [0, 1]

    int toInt_r() const; // retourne 0-255 pour l'écriture PPM
    int toInt_g() const;
    int toInt_b() const;
};

#endif /* !COLOR_HPP_ */
