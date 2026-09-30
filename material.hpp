#pragma once
#include "geometry.hpp"

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
  double albedo;

  PhongMaterial(const Vec3f &color, const double a, const double se)
      : diffuse_color(color), specular_exponent(se), albedo(a) {};
  PhongMaterial() : diffuse_color(), specular_exponent(), albedo() {};
};
