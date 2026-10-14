#include "include/env_map.hpp"
#include "include/geometry.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <algorithm>
#include <cmath>
#include <iostream>

extern unsigned int width;
extern unsigned int height;

Env_map::Env_map(std::string p) : path(p) {
  unsigned char *img = stbi_load("assets/envmap.jpg", &m_width, &m_height, &m_bpp, 3);

  if (img == NULL) {
    std::cerr << "Envmap not open" << std::endl;
    return;
  }

  Env_map::img = img;

  std::cout << "envmap " << p << " loaded : " << m_width << " " << m_height << " " << m_bpp
            << std::endl;
}

void Env_map::free_img() { stbi_image_free(img); }

Vec3f Env_map::get_sample(Vec3f &dir) {

  double u = 0.5f + std::atan2(dir.z, dir.x) / (2 * M_PI);
  double v = 1 - (std::asin(dir.y) / M_PI + .5);

  int x = std::min(int(u * m_width), int(m_width - 1));
  int y = std::min(int(v * m_height), int(m_height - 1));

  int idx = (x + y * m_width) * m_bpp;

  return Vec3f(img[idx] / 255., img[idx + 1] / 255., img[idx + 2] / 255.);
}
