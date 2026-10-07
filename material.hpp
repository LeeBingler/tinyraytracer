#pragma once
#include "geometry.hpp"
#include <array>

class Material {
public:
  Material(const Vec3f &color) : diffuse_color(color) {}
  Material() : diffuse_color() {}
  Vec3f diffuse_color;
};

class PhongMaterial : Material {
public:
  Vec3f diffuse_color;
  std::array<double, 4> albedo;
  double specular_exponent;
  double refractive_index;

  PhongMaterial(const Vec3f &color, const std::array<double, 4> &a, const double &se,
                const double &r)
      : diffuse_color(color), albedo(a), specular_exponent(se), refractive_index(r) {};
  PhongMaterial() : diffuse_color(), albedo(), specular_exponent(), refractive_index() {};
};
