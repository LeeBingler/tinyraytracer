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

Vec3f whiteColor(1., 1., 1.);
Vec3f blueColor(.0, .0, 1.);
Vec3f redColor(1., 0., 0.);
Vec3f BgColor(0.2, 0.2, 0.2);

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

Vec3f reflect(Vec3f vec, Vec3f N) { return vec - N * 2.f * (vec * N); }

Vec3f cast_ray(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, std::vector<Light> lights,
               size_t depth = 0) {
  PhongMaterial material;
  Vec3f normal, hit;

  if (depth > 4 || !spheres_intersect(ori, dir, spheres, material, normal, hit)) {
    return BgColor;
  }

  // TODO: Take everything here to a PhongMaterial method that return only the final pixel color

  // Mirror reflections
  Vec3f reflect_dir = reflect(dir, normal).normalize();
  // offset the original point to avoid occlusion by the object itself
  Vec3f reflect_orig = reflect_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
  Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

  double diffuse_light = .0;
  double specular_light = .0;

  for (auto light : lights) {
    Vec3f light_dir = (light.position - hit).normalize();
    Vec3f reflection_light = reflect(-light_dir, normal);

    // shadow receive
    double light_length = (light.position - hit).norm();
    // move the hit point along N because hit is on the surface of the sphere and can intersect
    // with himself
    Vec3f shadow_orig = light_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
    Vec3f shadow_normal, shadow_hit;
    PhongMaterial tmpMaterial;

    if (spheres_intersect(shadow_orig, light_dir, spheres, tmpMaterial, shadow_normal,
                          shadow_hit) &&
        (shadow_hit - shadow_orig).norm() < light_length)
      continue;

    diffuse_light += light.intensity * std::max(0.f, light_dir * normal);
    specular_light += light.intensity * std::powf(std::max(0.f, reflection_light * (-dir)),
                                                  material.specular_exponent);
  }

  // Sphere color at dir
  return material.diffuse_color * diffuse_light * material.albedo[0] +
         whiteColor * specular_light * material.albedo[1] + reflect_color * material.albedo[2];
}

void render(std::vector<Vec3f> &framebuffer, std::vector<Sphere> &spheres,
            std::vector<Light> lights) {
#pragma omp parallel for
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

  PhongMaterial red{redColor, {.6, 0.3, .3}, 50.};
  PhongMaterial blue{blueColor, {.9, .1, .1}, 10.};
  PhongMaterial mirror{Vec3f(1., 1., 1.), {0., 10., .8}, 1425.};

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
  lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
  lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, red));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, blue));
  spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, blue));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

  render(framebuffer, spheres, lights);

  save_image(framebuffer);
  return 0;
}
