#include "include/geometry.hpp"
#include "include/material.hpp"
#include "include/object.hpp"
#include "include/save_image.hpp"
#include "include/scene_intersect.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>

extern constexpr unsigned int width = 1024;
extern constexpr unsigned int height = 768;
constexpr double fov = M_PI / 3.;

Vec3f whiteColor(1., 1., 1.);
Vec3f blueColor(.0, .0, 1.);
Vec3f redColor(1., 0., 0.);
Vec3f BgColor(0.2, 0.2, 0.2);

Vec3f reflect(Vec3f vec, Vec3f &N) { return vec - N * 2.f * (vec * N); }

// Snell's law
Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) {
  float cosi = -std::max(-1.f, std::min(1.f, I * N));
  float etai = 1, etat = refractive_index;
  Vec3f n = N;

  // if the ray is inside the object, swap the indices and invert the normal to get the correct
  // result
  if (cosi < 0) {
    cosi = -cosi;
    std::swap(etai, etat);
    n = -N;
  }
  float eta = etai / etat;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  return k < 0 ? Vec3f(0, 0, 0) : I * eta + n * (eta * cosi - sqrtf(k));
}

Vec3f cast_ray(Vec3f ori, Vec3f &dir, std::vector<Sphere> &spheres, std::vector<Light> lights,
               size_t depth = 0) {
  PhongMaterial material;
  Vec3f normal, hit;

  if (depth > 4 || !scene_intersect(ori, dir, spheres, material, normal, hit))
    return BgColor;

  // Mirror reflections
  Vec3f reflect_dir = reflect(dir, normal).normalize();
  // offset the original point to avoid occlusion by the object itself
  Vec3f reflect_orig = reflect_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
  Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

  // Mirror refraction
  Vec3f refraction_dir = refract(dir, normal, material.refractive_index).normalize();
  // offset the original point to avoid occlusion by the object itself
  Vec3f refraction_orig = refraction_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
  Vec3f refraction_color = cast_ray(refraction_orig, refraction_dir, spheres, lights, depth + 1);

  double diffuse_light = .0;
  double specular_light = .0;

  for (auto light : lights) {
    Vec3f light_dir = (light.position - hit).normalize();
    Vec3f reflection_light = reflect(-light_dir, normal);

    // shadow receive
    double light_length = (light.position - hit).norm();
    // offset the original point to avoid occlusion by the object itself
    Vec3f shadow_orig = light_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
    Vec3f shadow_normal, shadow_hit;
    PhongMaterial tmpMaterial;

    if (scene_intersect(shadow_orig, light_dir, spheres, tmpMaterial, shadow_normal, shadow_hit) &&
        (shadow_hit - shadow_orig).norm() < light_length)
      continue;

    diffuse_light += light.intensity * std::max(0.f, light_dir * normal);
    specular_light += light.intensity * std::powf(std::max(0.f, reflection_light * (-dir)),
                                                  material.specular_exponent);
  }

  // Sphere color at dir
  return material.diffuse_color * diffuse_light * material.albedo[0] +
         whiteColor * specular_light * material.albedo[1] + reflect_color * material.albedo[2] +
         refraction_color * material.albedo[3];
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

int main() {
  std::vector<Vec3f> framebuffer(width * height);

  PhongMaterial red{redColor, {.6, .7, .1, .0}, 50., 1.};
  PhongMaterial rubber_blue{blueColor, {.9, .1, .0, .0}, 7., 1.};
  PhongMaterial mirror{Vec3f(1., 1., 1.), {.0, 10., .8, .0}, 1425., 1.};
  PhongMaterial glass{Vec3f(1., 1., 1.), {.0, .5, .1, .8}, 125., 1.5};

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
  lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
  lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, red));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, glass));
  spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, rubber_blue));
  spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

  render(framebuffer, spheres, lights);

  save_image(framebuffer);
  return 0;
}
