#include "include/scene_intersect.hpp"

double checkboard_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres,
                            PhongMaterial &material, Vec3f &normal, Vec3f &hit,
                            double &dist_min_scene) {
  double checkerboard_dist = std::numeric_limits<float>::max();

  if (fabs(dir.y) > 1e-3) {
    // the checkerboard plane has equation y = -4
    double d = -(ori.y + 4) / dir.y;
    Vec3f pt = ori + dir * d;

    if (d > 0 && fabs(pt.x) < 10 && pt.z < -10 && pt.z > -30 && d < dist_min_scene) {
      checkerboard_dist = d;
      hit = pt;
      normal = Vec3f(0, 1, 0);
      material.diffuse_color =
          (int(.5 * hit.x + 1000) + int(.5 * hit.z)) & 1 ? Vec3f(1., 1., 1.) : Vec3f(.1, 1., .1);
      material.diffuse_color = material.diffuse_color * .3;
      material.specular_exponent = 50.; // keep a tight highlight
    }
  }

  return checkerboard_dist < dist_min_scene ? checkerboard_dist : dist_min_scene;
}

double spheres_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres,
                         PhongMaterial &material, Vec3f &normal, Vec3f &hit) {
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

  return spheres_dist;
}

bool scene_intersect(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, PhongMaterial &material,
                     Vec3f &normal, Vec3f &hit) {
  double dist_min_scene;

  dist_min_scene = spheres_intersect(ori, dir, spheres, material, normal, hit);
  dist_min_scene = checkboard_intersect(ori, dir, spheres, material, normal, hit, dist_min_scene);

  return dist_min_scene < 1000;
}
