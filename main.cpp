#include "geometry.hpp"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

constexpr int width = 1024;
constexpr int height = 768;
constexpr double fov = M_PI / 2.;

class Material {
public:
  Material(const Vec3f &color) : diffuse_color(color) {}
  Material() : diffuse_color() {}
  Vec3f diffuse_color;
};

class Sphere {
public:
  Vec3f position;
  double radius;
  Material material;
  Sphere(const Vec3f &p, const double &r, const Material &m)
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

bool spheres_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres,
                       Material &material) {
  double spheres_dist = std::numeric_limits<float>::max();

  for (auto sphere : spheres) {
    double dist_i;
    if (sphere.ray_intersect(ori, dir, dist_i) && dist_i < spheres_dist) {
      spheres_dist = dist_i;
      material = sphere.material;
    }
  }

  return spheres_dist < 1000;
}

Vec3f cast_ray(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres) {
  Material material;

  if (spheres_intersect(ori, dir, spheres, material))
    return material.diffuse_color; // Sphere color

  return Vec3f(0.7, 0.7, 0.7); // Background color
}

void render(std::vector<Vec3f> &framebuffer, std::vector<Sphere> &spheres) {
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2. * (i + 0.5) / (float)width - 1.) * tan(fov / 2.) * width /
                (float)height;
      float y = -(2. * (j + 0.5) / (float)height - 1.) * tan(fov / 2.);
      Vec3f dir = Vec3f(x, y, -1.).normalize();

      framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres);
    }
  }
}

void save_image(std::vector<Vec3f> &framebuffer) {
  std::ofstream ofs; // save the framebuffer to file
  ofs.open("./out.ppm");
  ofs << "P6\n" << width << " " << height << "\n255\n";

  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

int main() {
  std::vector<Vec3f> framebuffer(width * height);

  Material red{Vec3f(1.0, .0, .0)};
  Material blue{Vec3f(.0, .0, 1.0)};

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, red));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red));
  spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, blue));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, red));

  render(framebuffer, spheres);

  save_image(framebuffer);
  return 0;
}
