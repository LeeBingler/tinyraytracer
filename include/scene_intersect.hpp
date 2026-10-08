#pragma once

#include "geometry.hpp"
#include "material.hpp"
#include "object.hpp"

double checkboard_intersect(Vec3f ori, Vec3f dir, PhongMaterial &material, Vec3f &normal,
                            Vec3f &hit, double &dist_min_scene);

double spheres_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres,
                         PhongMaterial &material, Vec3f &normal, Vec3f &hit);

bool scene_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, PhongMaterial &material,
                     Vec3f &normal, Vec3f &hit);
