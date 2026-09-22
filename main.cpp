#include "geometry.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

struct Sphere {
  Vec3f position;
  float radius;

  Sphere(const Vec3f &p, const float &r) : position(p), radius(r) {}

  // Ray intersect doc:
  // https://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
  bool ray_intersect(Vec3f &origin, Vec3f &direction) {
    Vec3f hypo = origin - position;
    float projP = hypo * direction;
    float perp_square = hypo * hypo - projP * projP;

    if (perp_square > radius * radius)
      return false;

    return true;
  }
};

void render() {
  const int width = 1024;
  const int height = 768;
  std::vector<Vec3f> framebuffer(width * height);

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      framebuffer[i + j * width] =
          Vec3f(j / float(height), i / float(width), 0);
    }
  }

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
  render();
  return 0;
}
