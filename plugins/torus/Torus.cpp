#include "Torus.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

Torus::Torus(const Point3D& center, double r1, double r2, const Color& color,
             double transparency, double refractiveIndex, double reflectivity,
             double shininess, const Color& specularColor)
    : _center(center), _r1(r1), _r2(r2), _color(color),
      _transparency(transparency), _refractiveIndex(refractiveIndex),
      _reflectivity(reflectivity), _shininess(shininess),
      _specularColor(specularColor)
{
}

int Torus::solveQuadric(double c[3], double s[2])
{
    double a = c[2];
    double b = c[1];
    double cc = c[0];

    if (std::abs(a) < 1e-9) {
        if (std::abs(b) < 1e-9)
            return 0;
        s[0] = -cc / b;
        return 1;
    }

    double disc = b * b - 4.0 * a * cc;
    if (disc < 0.0)
        return 0;
    if (disc == 0.0) {
        s[0] = -b / (2.0 * a);
        return 1;
    }

    double sqrtD = std::sqrt(disc);
    s[0] = (-b - sqrtD) / (2.0 * a);
    s[1] = (-b + sqrtD) / (2.0 * a);
    return 2;
}

int Torus::solveCubic(double c[4], double s[3])
{
    int numRoots = 0;
    double a = c[3];
    double b = c[2];
    double cc = c[1];
    double d = c[0];

    if (std::abs(a) < 1e-9) {
        double q_coeff[3] = {d, cc, b};
        return solveQuadric(q_coeff, s);
    }

    // Convert to monic: x^3 + Bx^2 + Cx + D = 0
    double B = b / a;
    double C = cc / a;
    double D = d / a;

    // Depressed cubic: y^3 + py + q = 0, where x = y - B/3
    double p = C - B * B / 3.0;
    double q = D - B * C / 3.0 + 2.0 * B * B * B / 27.0;

    double disc = (q * q / 4.0) + (p * p * p / 27.0);

    if (disc > 1e-9) {
        // One real root
        double sqrtD = std::sqrt(disc);
        double u = std::cbrt(-q / 2.0 + sqrtD);
        double v = std::cbrt(-q / 2.0 - sqrtD);
        s[0] = u + v - B / 3.0;
        numRoots = 1;
    } else if (std::abs(disc) <= 1e-9) {
        // Three real roots, at least two equal
        if (std::abs(p) < 1e-9) {
            s[0] = -B / 3.0;
            numRoots = 1;
        } else {
            double u = std::cbrt(-q / 2.0);
            s[0] = 2.0 * u - B / 3.0;
            s[1] = -u - B / 3.0;
            numRoots = 2; // Unique roots returned
        }
    } else {
        // Three distinct real roots
        double r = std::sqrt(-p * p * p / 27.0);
        double phi = std::acos(std::clamp(-q / (2.0 * r), -1.0, 1.0));
        double factor = 2.0 * std::sqrt(-p / 3.0);
        s[0] = factor * std::cos(phi / 3.0) - B / 3.0;
        s[1] = factor * std::cos((phi + 2.0 * M_PI) / 3.0) - B / 3.0;
        s[2] = factor * std::cos((phi + 4.0 * M_PI) / 3.0) - B / 3.0;
        numRoots = 3;
    }

    return numRoots;
}

int Torus::solveQuartic(double c[5], double s[4])
{
    double a = c[4];
    double b = c[3];
    double cc = c[2];
    double d = c[1];
    double e = c[0];

    if (std::abs(a) < 1e-9) {
        double c_coeff[4] = {e, d, cc, b};
        return solveCubic(c_coeff, s);
    }

    // Convert to monic: x^4 + Ax^3 + Bx^2 + Cx + D = 0
    double A = b / a;
    double B = cc / a;
    double C = d / a;
    double D = e / a;

    // Depressed quartic: y^4 + p y^2 + q y + r = 0, where x = y - A/4
    double p = B - 3.0 * A * A / 8.0;
    double q = C - A * B / 2.0 + A * A * A / 8.0;
    double r = D - A * C / 4.0 + A * A * B / 16.0 - 3.0 * A * A * A * A / 256.0;

    int numRoots = 0;

    if (std::abs(q) < 1e-9) {
        // Biquadratic: y^4 + p y^2 + r = 0
        double q_coeff[3] = {r, p, 1.0};
        double roots_v[2];
        int num_v = solveQuadric(q_coeff, roots_v);
        for (int i = 0; i < num_v; ++i) {
            if (roots_v[i] >= 0.0) {
                double sqrtV = std::sqrt(roots_v[i]);
                s[numRoots++] = sqrtV - A / 4.0;
                s[numRoots++] = -sqrtV - A / 4.0;
            }
        }
        return numRoots;
    }

    // Resolvent cubic: z^3 - p z^2 - 4r z + 4pr - q^2 = 0
    double cubic_c[4] = {4.0 * p * r - q * q, -4.0 * r, -p, 1.0};
    double cubic_s[3];
    int numCubicRoots = solveCubic(cubic_c, cubic_s);

    if (numCubicRoots == 0)
        return 0;

    // Find the largest real root of the resolvent cubic
    double z = cubic_s[0];
    for (int i = 1; i < numCubicRoots; ++i) {
        if (cubic_s[i] > z)
            z = cubic_s[i];
    }

    double d_val = z - p;
    if (d_val < 0.0) {
        if (d_val > -1e-9)
            d_val = 0.0;
        else
            return 0;
    }

    double a_prime = std::sqrt(d_val);
    
    // First quadratic: y^2 + a_prime y + b1 = 0
    double b1 = z / 2.0 - q / (2.0 * a_prime);
    double disc1 = a_prime * a_prime - 4.0 * b1;
    if (disc1 >= 0.0) {
        double sqrtD = std::sqrt(disc1);
        s[numRoots++] = (-a_prime + sqrtD) / 2.0 - A / 4.0;
        s[numRoots++] = (-a_prime - sqrtD) / 2.0 - A / 4.0;
    }

    // Second quadratic: y^2 - a_prime y + b2 = 0
    double b2 = z / 2.0 + q / (2.0 * a_prime);
    double disc2 = a_prime * a_prime - 4.0 * b2;
    if (disc2 >= 0.0) {
        double sqrtD = std::sqrt(disc2);
        s[numRoots++] = (a_prime + sqrtD) / 2.0 - A / 4.0;
        s[numRoots++] = (a_prime - sqrtD) / 2.0 - A / 4.0;
    }

    return numRoots;
}

std::optional<double> Torus::hits(const Ray& ray) const
{
    // Translate the ray to the local space of the torus (centered at _center)
    Vector3D O = ray.origin - _center;
    Vector3D D = ray.direction.normalize();

    double R = _r1;
    double r = _r2;

    double x0 = O.X, y0 = O.Y, z0 = O.Z;
    double dx = D.X, dy = D.Y, dz = D.Z;

    double H = x0 * dx + y0 * dy + z0 * dz;
    double K = x0 * x0 + y0 * y0 + z0 * z0 + R * R - r * r;

    double D_xz = dx * dx + dz * dz;
    double O_xz = x0 * dx + z0 * dz;
    double S_xz = x0 * x0 + z0 * z0;

    // Coefficients of the quartic equation: c4*t^4 + c3*t^3 + c2*t^2 + c1*t + c0 = 0
    // Since D is normalized, c4 = (D.dot(D))^2 = 1.0
    double c[5];
    c[4] = 1.0;
    c[3] = 4.0 * H;
    c[2] = 2.0 * K + 4.0 * H * H - 4.0 * R * R * D_xz;
    c[1] = 4.0 * H * K - 8.0 * R * R * O_xz;
    c[0] = K * K - 4.0 * R * R * S_xz;

    double roots[4];
    int numRoots = solveQuartic(c, roots);

    if (numRoots == 0)
        return std::nullopt;

    constexpr double EPSILON = 1e-4;
    double rayDirLen = ray.direction.length();
    double min_t_root = -1.0;

    for (int i = 0; i < numRoots; ++i) {
        if (roots[i] > EPSILON * rayDirLen) {
            if (min_t_root < 0.0 || roots[i] < min_t_root) {
                min_t_root = roots[i];
            }
        }
    }

    if (min_t_root < 0.0)
        return std::nullopt;

    return min_t_root / rayDirLen;
}

Vector3D Torus::getNormal(const Point3D& point) const
{
    // Translate the point to local space
    Vector3D p = point - _center;
    double d = std::sqrt(p.X * p.X + p.Z * p.Z);

    if (d < 1e-9) {
        // Fallback for points exactly on the y-axis
        return Vector3D(0.0, p.Y > 0.0 ? 1.0 : -1.0, 0.0);
    }

    Vector3D normal(
        p.X * (1.0 - _r1 / d),
        p.Y,
        p.Z * (1.0 - _r1 / d)
    );

    return normal.normalize();
}

Color Torus::getColor(const Point3D& point) const
{
    (void)point;
    return _color;
}

double Torus::getOpacity() const
{
    return 1.0 - _transparency;
}

double Torus::getRefractiveIndex() const
{
    return _refractiveIndex;
}

double Torus::getReflectivity() const
{
    return _reflectivity;
}

double Torus::getShininess() const
{
    return _shininess;
}

Color Torus::getSpecularColor() const
{
    return _specularColor;
}

extern "C" IPrimitive* createPrimitive(const std::map<std::string, double>& params)
{
    Point3D center(params.at("x"), params.at("y"), params.at("z"));
    double  r1 = params.at("r1");
    double  r2 = params.at("r2");
    double  transparency = params.count("transparency") ? params.at("transparency") : 0.0;
    double  refractiveIndex = params.count("refractiveIndex") ? params.at("refractiveIndex") : 1.0;
    double  reflectivity = params.count("reflectivity") ? params.at("reflectivity") : 0.0;
    Color   color(params.at("color_r"), params.at("color_g"), params.at("color_b"));
    double  shininess = params.count("shininess") ? params.at("shininess") : 0.0;
    Color   spec(1, 1, 1);
    if (params.count("spec_r") && params.count("spec_g") && params.count("spec_b")) {
        spec = Color(params.at("spec_r"), params.at("spec_g"), params.at("spec_b"));
    }

    return new Torus(center, r1, r2, color, transparency, refractiveIndex, reflectivity, shininess, spec);
}

extern "C" void destroyPrimitive(IPrimitive* p)
{
    delete p;
}
