#include "geometry.hpp"
#include "material.hpp"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

constexpr int width = 1024;
constexpr int height = 768;
constexpr double fov = M_PI / 2.;

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

bool spheres_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, PhongMaterial &material,
                       Vec3f &normal, Vec3f &hit) {
  double spheres_dist = std::numeric_limits<double>::max();

  for (auto sphere : spheres) {
    double dist_i;
    if (sphere.ray_intersect(ori, dir, dist_i) && dist_i < spheres_dist) {
      spheres_dist = dist_i;
      hit = ori + dir * dist_i;
      normal = (hit - sphere.position).normalize();
      material = sphere.material;
    }
  }

  return spheres_dist < 1000;
}

Vec3f cast_ray(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, std::vector<Light> lights) {
  PhongMaterial material;
  Vec3f normal, hit;

  if (spheres_intersect(ori, dir, spheres, material, normal, hit)) {
    double diffuse_light = .0;
    double specular_light = .0;

    for (auto light : lights) {
      Vec3f light_dir = (light.position - hit).normalize();
      // Vec3f reflection_light = normal * (normal *)diffuse_light;

      diffuse_light += light.intensity * std::max(0.f, light_dir * normal);
      specular_light += light.intensity * std::max(0.f, light_dir * normal);
    }

    return material.diffuse_color * (diffuse_light + specular_light); // Sphere color
  }

  return Vec3f(0.7, 0.7, 0.7); // Background color
}

void render(std::vector<Vec3f> &framebuffer, std::vector<Sphere> &spheres,
            std::vector<Light> lights) {

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2. * (i + 0.5) / (float)width - 1.) * tan(fov / 2.) * width / (float)height;
      float y = -(2. * (j + 0.5) / (float)height - 1.) * tan(fov / 2.);
      Vec3f dir = Vec3f(x, y, -1.).normalize();

      framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
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

  PhongMaterial red{Vec3f(1.0, .0, .0), 0.3, 50.};
  PhongMaterial blue{Vec3f(.0, .0, 1.0), 0.9, 10.};

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, red));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red));
  spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, blue));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, red));

  render(framebuffer, spheres, lights);

  save_image(framebuffer);
  return 0;
}
