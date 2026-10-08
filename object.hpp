#pragma once

#include "geometry.hpp"
#include "material.hpp"

class Sphere {
public:
  Vec3f position;
  double radius;
  PhongMaterial material;

  Sphere(const Vec3f &p, const double &r, const PhongMaterial &m)
      : position(p), radius(r), material(m) {}

  // Ray intersect doc:
  // https://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
  bool ray_intersect(const Vec3f &origin, const Vec3f &direction, double &t0) {
    Vec3f hypo = position - origin;
    double projP = hypo * direction;
    double perp_square = hypo * hypo - projP * projP;

    // Check if ray intersect with the circle
    if (perp_square > radius * radius)
      return false;

    double thc = sqrtf(radius * radius - perp_square);
    t0 = projP - thc;
    double t1 = projP + thc;

    // check if the sphere is behind ray (x-axis in 2D)
    if (t0 < 0)
      t0 = t1;
    if (t0 < 0)
      return false;

    return true;
  }
};

class Light {
public:
  Vec3f position;
  double intensity;
  Light(const Vec3f &p, const double &i) : position(p), intensity(i) {};
};
