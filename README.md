Documentation: https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing

# Step 1 : display the image
To do this, he write an image to the disk. We can improve that with GLAD to open a window and display our image in real time but I wont do it for now to follow step by step his tutorial.

# Step 2 : Ray Tracing

We want to define a sphere inside the image. To do that we define a sphere in 3D, we only need its origin and the radius of the sphere. In addition to that, he add a ray_intersect function to know if a ray intersect with the sphere at all (A detailed description of the algorithm for the ray-sphere intersection [can be found here](http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/)) :
```cpp
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
```

To do a raytracing, we shoot FOR EACH PIXEL a ray from the camera and see if the ray intersect with the sphere. If it intersect , we know there is a sphere here so we change color :
```cpp
Vec3f cast_ray(Vec3f ori, Vec3f dir, Sphere &sphere) {
  float sphere_dist = std::numeric_limits<float>::max();
  if (sphere.ray_intersect(ori, dir, sphere_dist)) {
    return Vec3f(0.2, 0.7, 0.8); // Background color
  }

  return Vec3f(0.4, 0.4, 0.3); // Normal color
}

void render(std::vector<Vec3f> &framebuffer, Sphere &sphere) {
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
    // pixel coord (i,j) to world coord (x,y)
      float x = (2. * (i + 0.5) / (float)width - 1.) * tan(fov / 2.) * width /
                (float)height; 
      float y = -(2. * (j + 0.5) / (float)height - 1.) * tan(fov / 2.);
      // We shoot ray straight in z-axis from the camera
      Vec3f dir = Vec3f(x, y, -1.).normalize();

      framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, sphere);
    }
  }
}
```


To understand how we change from pixel to world coordinate for the direction of the ray.
```cpp
float x = (2. * (i + 0.5) / (float)width - 1.) * tan(fov / 2.) * width / 
		  (float)height; 
float y = -(2. * (j + 0.5) / (float)height - 1.) * tan(fov / 2.);
```

Here is a representation of the camera from the top, in our image the screen is 16 pixels wide:
![[trace.png]]

The `z = -1` because we want to shoot ray to the scene from the origin (camera `Vec3f(0,0,0)`).

The `fov` specify what sector of the screen will be visible. (here 2 / PI)

To compute the length of the screen in world coordinate, we have to use Pythagoras theorem. 
Let us focus on the triangle formed by the red, gray and gray dashed line. It is easy to see that `tan(field of view / 2) = (screen width) * 0.5 / (screen-camera distance).` We placed the screen at the distance of 1 from the camera, thus `(screen width) = 2 * tan(field of view / 2)`.

Now let us say that we want to cast a vector through the center of the 12th pixel of the screen, i.e. we want to compute the blue vector. How can we do that? What is the distance from the left of the screen to the tip of the blue vector? First of all, it is 12+0.5 pixels. We know that 16 pixels of the screen correspond to `2 * tan(fov/2)` world units. Thus, tip of the vector is located at `(12+0.5) / 16 * 2 * tan(fov/2)` world units from the left edge, or at the distance of `(12+0.5) * 2/16 * tan(fov/2) - tan(fov/2)` from the intersection between the screen and the -z axis. Add the screen **aspect ratio** to the computations and you will find exactly the formulas for the ray direction.

# Step 3 : add more spheres
Make an vector of spheres and tweak the code to make it work.

# Step 4 : Lighting

Let's add few point light sources:
``` cpp
struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};
```

Now we want to add the light intensity to the color of the sphere. To do that we need to to the dot product of the light direction ray (between the light and the point hit) AND the normal in tangent space of the sphere. After that we multiply the intensity of the current light by the dot product to get an intensity for the sphere and add it to the current color :
``` cpp
// calculation of the normal and the point where the sphere is hit
if (sphere.ray_intersect(ori, dir, dist_i) && dist_i < spheres_dist) {
	spheres_dist = dist_i;
    hit = ori + dir * dist_i;
    normal = (hit - sphere.position).normalize();
    material = sphere.material;
}
```

``` cpp
  if (spheres_intersect(ori, dir, spheres, material, normal, hit)) {
    double lights_intensity = .0;

    for (auto light : lights) {
	  // dot product to get the lights intensity 
      Vec3f light_dir = (light.position - hit).normalize();
      lights_intensity += light.intensity * std::max(0.f, light_dir * normal);
    }

    return material.diffuse_color * lights_intensity; // Sphere color
}
```

#REMINDER: The dot product (produit scalaire) is use between 2 vector to get the angle (cos) between them.

# Step 5 : Specular lighting
![[68747470733a2f2f75706c6f61642e77696b696d656469612e6f72672f77696b6970656469612f636f6d6d6f6e732f362f36622f50686f6e675f636f6d706f6e656e74735f76657273696f6e5f342e706e67.png]]

To make the specular lighting, we calculate the reflection of the light on the surface and see how much it align with the camera.

The more it align, the more the specular is showing in white:
```cpp
Vec3f reflect(Vec3f vec, Vec3f N) { return vec - N * 2.f * (vec * N); }

Vec3f cast_ray(Vec3f ori, Vec3f dir, std::vector<Sphere> &spheres, std::vector<Light> lights) {
  PhongMaterial material;
  Vec3f normal, hit;

  if (spheres_intersect(ori, dir, spheres, material, normal, hit)) {
    double diffuse_light = .0;
    double specular_light = .0;

    for (auto light : lights) {
      Vec3f light_dir = (light.position - hit).normalize();
      Vec3f reflection_light = reflect(-light_dir, normal);

      diffuse_light += light.intensity * std::max(0.f, light_dir * normal);
      specular_light += light.intensity * std::powf(std::max(0.f, reflection_light * (-dir)), material.specular_exponent);
    }

    // Sphere color at dir
    return material.diffuse_color * diffuse_light * material.albedo[0] +
           whiteColor * specular_light * material.albedo[1];
  }

  return BgColor;
}

```

The calculation for the reflection of light is like that:

What is reflection of a vector V:
$$
V = \begin{pmatrix} x = Vparallel \\ y = V perpendicular \end{pmatrix}, 
V_{reflection} = \begin{pmatrix} x = Vparallel \\ y = -V perpendicular \end{pmatrix}
$$

How to find the x value of vector V (the vector that is parallel to the abscissa):
$$
Vec \cdot Normal = projection Of Vec Along Normal = shadowLength
$$
$$
Normal \cdot shadowLength = VecAlongNormalWithShadowLength = V_{perp}
$$
How to find the y value of vector V (vector that is perpendicular to the abscissa):
$$
V = V_{parallel} + V_{perpendicular} \rightarrow V_{para} = V - V_{perp}
$$

The calculation to find the reflection:
$$
V_{reflection} = V_{para} - V_{perp} \rightarrow V_{refl} = (V - V_{perp}) - V_{perp} \rightarrow V_{refl} = V - 2 * V_{perp}
$$
$$
V_{refl} = V - 2 * N * (V * N)
$$


# Step 6 : Shadow

To make receiving shadow on the material, we make sure that the segment between the hit point of the sphere and the light does not intersect with an object of the scene. If there is an intersection, then there is a shadow so we skip the light source.

We move the hit point by the normal point a little to avoid the hit point intersection with himself:
```cpp
// shadow receive
double light_length = (light.position - hit).norm();
// move the hit point along N because hit is on the surface of the sphere and can intersect
// with himself
Vec3f shadow_orig = light_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
Vec3f shadow_normal, shadow_hit;
PhongMaterial tmpMaterial;

if (spheres_intersect(shadow_orig, light_dir, spheres, tmpMaterial, shadow_normal,
    shadow_hit) && (shadow_hit - shadow_orig).norm() < light_length)
        continue;
```

# Step 7 : Reflections
To add more realism, objects always have reflection of thing around them (more or less).

To do that, we take the code from the shadow receive and cast a ray from the hit point to the reflection direction and see what render in hit. We literally are rendering the scene inside the sphere to do a reflection. We do it back and forth between all point in the scene so we limit the reflection to 4:
```cpp
Vec3f reflect_dir = reflect(dir, normal).normalize();
// offset the original point to avoid occlusion by the object itself
Vec3f reflect_orig = reflect_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3;
Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

// ...

return material.diffuse_color * diffuse_light * material.albedo[0] +
         whiteColor * specular_light * material.albedo[1] + reflect_color * material.albedo[2];

```

# Step 8 : Refraction
We need to add one function to compute the refracted ray ([using Snell's law](https://en.wikipedia.org/wiki/Snell%27s_law)), and three more lines of code in our recursive function cast_ray.

Watch this to understand snell's law: https://www.youtube.com/watch?v=Iq1a_KJTWJ8

Snells law say that :
![[Screenshot_20261007_161238.png]]

Where Vair and Vwater are the speed of light inside air and water.
Vwater can be any medium (matter where light travel).

And that:
![[Screenshot_20261007_164208.png]]

To our case, we need to find the angle for $sin(i)$  and the refractive indices of our matter $n_1$ and $n_2$ to find $sin(r)$ and use cast ray with it. 
We can decide the refractive indices arbitrarily, for our matter the user will decide and for the light it will be 1.

#### Step 1 : To find the angle incidence angle `i`:
1. We do the dot product between the normal and the vector and put it at negative to get $cos(i)$
2. We clamp this angle between -1 and 1 (???)

#### Step 2 : Entering or leaving? :
If the angle is negative, it means we are leaving the matter.
Because the vector and the normal are pointing the same direction so the angle was positive and because we put it at the inverse it is now negative, so leaving the matter.

So we invert the angle and the normal. Then swap `n1 and n2` for our calculus. 

#### Step 3 : Snell's law:
Now we have our refractive indices `etai and etat`, the angle incidence() from the light to the surface) by our normal `cosi`. We need to get the angle inside our matter now $sin(r)$

`etai` is the refractive index of the medium the ray is currently in. `etat` is the index of the medium it is entering.

Snell's law says `etai · sin θi = etat · sin θt`. 
With `eta = etai / etat`, this is `eta · sin θi = sin θt`.
```cpp
float k = 1 - eta*eta*(1 - cosi*cosi);
```

Since `1 - cosi²` is `sin²θi`, we get `k = 1 - sin²θt = cos²θt`. 
So `k` is the squared cosine of the refracted angle $cos^2(r)$.

If `k < 0`, then `sin θt` would have to exceed 1, which is impossible. That is **total internal reflection**: the ray cannot exit, and the function returns the zero vector to signal it.

#### Step 4 : build the refracted direction
Split the incoming ray relative to `n`:
- The part along the surface is `I + cosi·n`. It has length `sin θi`.
- The part along the normal is `-cosi·n`.

Refraction keeps the direction of the surface part but rescales it to length `sin θt = eta · sin θi`, so it is multiplied by `eta`. The normal part becomes `-cosθt · n`, where `cosθt = sqrt(k)`. Adding them gives:
`T = eta·(I + cosi·n) - sqrt(k)·n = eta·I + n·(eta·cosi - sqrt(k))`

# Step 9 : Add checkboard
Nothing to explain. He just drop code and say try it...


# Step 10 : Homework

## Add Envmap :

We need to load the envmap first. To do that, I've use the `stb_image` library.



Then, if the ray doesn't intersect with an object, we display the envmap. 

The envmap is a 360° Photos.

To understand deeply how Rendering 360° Photos work : https://ikyle.me/blog/2026/render-360-equirectangular

To put it simply, the 360° Photos is warp the more your go to the corner:
![[equirectangular-grid.jpeg]]

An equirectangular image is essentially a world map projection applied to a sphere. It encodes `(longitude, latitude)` over `(u, v)` texture space:
- x coordinate : **Longitude** spans `[-π, +π]` horizontally (left edge = -180°, right edge = +180°)
- y coordinate : **Latitude** spans `[+π/2, -π/2]` vertically (top = +90°, bottom = -90°)

To get the right pixel, we need to do calculation on a direction vector. 
**Every pixel in the texture corresponds to a direction in 3D space**. The direction is pointing in a direction (in a 3D sphere), and we need to find the right pixel of the image that correspond to the direction in the sphere:
![[coordinate-axes.jpeg]]

To find the right coordinate point (latitude and longitude) with only a direction vector we need to use this (assuming the direction vector is normalize):
$$
latitude = asin(dir.y) \\
longitude = atan2(dir.z, dir.x)
$$

Now we have longitude and latitude that is between the range of the image.

Now we need to remap the value between 0 and 1 to get the UV:
$$
V = (latitude / PI) + 0.5 \\
U = (longitude / 2 * PI) + 0.5 
$$

#### Vertical coordinate - latitude (V):

With y up, `dir.y = sin(latitude)`, where latitude runs from -90° (straight down) to +90° (straight up). So:
- `asin(dir.y)` recovers the latitude, in [-π/2, π/2].
- Dividing by π gives [-0.5, 0.5], and adding 0.5 gives [0, 1].

This put our coordinate with 0 at the bottom and 1 at the top. And to reverse it we just need to make 1 - V: $v = 1 - (asin(dir.y)/π + 0.5) = 0.5 - asin(dir.y)/π$

#### Horizontal coordinate - longitude (U):

Longitude is the angle of the direction’s projection onto the horizontal plane. For y-up, that plane is `(x, z)`, and `atan2(z, x)` gives that angle:
- `atan2` returns the angle in (-π, π], covering the full circle.
- Dividing by 2π gives (-0.5, 0.5], and adding 0.5 gives (0, 1].

That gives us those formula:
$$
u = atan2(dir.z, dir.x) / (2 * PI) + 0.5
$$
$$
v = 1 - (asin(dir.y) / PI + 0.5)
$$


