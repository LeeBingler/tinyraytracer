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

struct Sphere {
  Vec3f position;
  double radius;

  Sphere(const Vec3f &p, const double &r) : position(p), radius(r) {}

  // Ray intersect doc:
  // https://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
  bool ray_intersect(Vec3f &origin, Vec3f &direction, float &t0) {
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

Vec3f cast_ray(Vec3f ori, Vec3f dir, Sphere &sphere) {
  float sphere_dist = std::numeric_limits<float>::max();
  if (sphere.ray_intersect(ori, dir, sphere_dist)) {
    return Vec3f(0.2, 0.7, 0.8); // Sphere color
  }

  return Vec3f(0.4, 0.4, 0.3); // Background color
}

void render(std::vector<Vec3f> &framebuffer, Sphere &sphere) {
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2. * (i + 0.5) / (float)width - 1.) * tan(fov / 2.) * width /
                (float)height;
      float y = -(2. * (j + 0.5) / (float)height - 1.) * tan(fov / 2.);
      Vec3f dir = Vec3f(x, y, -1.).normalize();

      framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, sphere);
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
  Sphere sphere(Vec3f(-3, 0, -16), 2);

  render(framebuffer, sphere);

  save_image(framebuffer);
  return 0;
}
