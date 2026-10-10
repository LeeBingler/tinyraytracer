#include "include/env_map.hpp"
#include "include/geometry.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include <iostream>

Env_map::Env_map(std::string p) : path(p) {
  unsigned char *img = stbi_load("assets/envmap.jpg", &m_width, &m_height, &m_bpp, 3);

  if (img == NULL) {
    std::cerr << "Envmap not open" << std::endl;
    return;
  }

  std::cout << m_width << " " << m_height << " " << m_bpp << std::endl;
}

void Env_map::free_img() { stbi_image_free(img); }

Vec3f Env_map::get_color(Vec3f &dir, Vec3f &ori) { return Vec3f(0.2, 0.2, 0.2); }
