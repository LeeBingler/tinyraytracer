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
  double specular_exponent;
  std::array<double, 3> albedo;

  PhongMaterial(const Vec3f &color, const std::array<double, 3> &a, const double se)
      : diffuse_color(color), specular_exponent(se), albedo(a) {};
  PhongMaterial() : diffuse_color(), specular_exponent(), albedo() {};
};
