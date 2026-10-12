#pragma once

#include "geometry.hpp"
#include <string>

class Env_map {
  std::string path;
  unsigned char *img;
  int m_width, m_height, m_bpp;

public:
  Env_map(std::string p);

  Vec3f get_sample(Vec3f &dir);

  void free_img();
};
