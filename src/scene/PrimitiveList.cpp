#include "PrimitiveList.hpp"

/*PrimitiveList::PrimitiveList()
{
}
*/
PrimitiveList::~PrimitiveList()
{
}

std::optional<double> PrimitiveList::hits(const Ray& ray) const
{
    std::optional<double> closest = std::nullopt;

    for (const IPrimitive* child : _children) {
        std::optional<double> t = child->hits(ray);

        if (t.has_value()) {
            if (!closest.has_value() || t.value() < closest.value())
                closest = t;
        }
    }

    return closest;
}

const IPrimitive* PrimitiveList::getClosest(const Ray& ray) const
{
    const IPrimitive* result   = nullptr;
    double minT = std::numeric_limits<double>::max();

    for (const IPrimitive* child : _children) {
        std::optional<double> t = child->hits(ray);

        if (t.has_value() && t.value() < minT) {
            minT   = t.value();
            result = child;
        }
    }

    return result;
}

void PrimitiveList::add(IPrimitive* primitive)
{
    _children.push_back(primitive);
}

Vector3D PrimitiveList::getNormal(const Point3D& point) const
{
    (void)point;
    return Vector3D(0, 0, 0);
}

Color PrimitiveList::getColor(const Point3D& point) const
{
    (void)point;
    return Color(0, 0, 0);
}

double PrimitiveList::getOpacity() const {
    return 1.0;
}

double PrimitiveList::getRefractiveIndex() const {
    return 1.0;
}

double PrimitiveList::getReflectivity() const {
    return 0.0;
}