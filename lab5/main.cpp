#include <tuple>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <cstdlib> // Для std::rand()
#include <stdexcept>
#include <iostream>
#include <ctime> // Для работы с временем
#include <sstream> // Для формирования имени файла


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct vec3 {
    float x=0, y=0, z=0;
          float& operator[](const int i)       { return i==0 ? x : (1==i ? y : z); }
    const float& operator[](const int i) const { return i==0 ? x : (1==i ? y : z); }
    vec3  operator*(const float v) const { return {x*v, y*v, z*v};       }
    float operator*(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }
    vec3  operator+(const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    vec3  operator-(const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    vec3  operator-()              const { return {-x, -y, -z};          }
    float norm() const { return std::sqrt(x*x+y*y+z*z); }
    vec3 normalized() const { return (*this)*(1.f/norm()); }
};

vec3 cross(const vec3 v1, const vec3 v2) {
    return { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

struct Material {
    float refractive_index = 1;
    float albedo[4] = {2,0,0,0};
    vec3 diffuse_color = {0,0,0};
    float specular_exponent = 0;
};

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

constexpr Material      ivory = {1.0, {0.9,  0.5, 0.1, 0.0}, {0.4, 0.4, 0.3},   50.};
constexpr Material      glass = {1.5, {0.0,  0.9, 0.1, 0.8}, {0.6, 0.7, 0.8},  125.};
constexpr Material red_rubber = {1.0, {1.4,  0.3, 0.0, 0.0}, {0.3, 0.1, 0.1},   10.};
constexpr Material     mirror = {1.0, {0.0, 16.0, 0.8, 0.0}, {1.0, 1.0, 1.0}, 1425.};
constexpr Material gold = {1.0, {0.5,  0.9, 0.1, 0.0}, {1.0, 0.85, 0.57}, 100.};
constexpr Material silver = {1.0, {0.5,  0.9, 0.2, 0.0}, {0.75, 0.75, 0.75}, 200.};
constexpr Material obsidian = {1.0, {0.7,  0.3, 0.1, 0.0}, {0.05, 0.05, 0.05}, 300.};
constexpr Material bronze = {1.0, {0.6,  0.6, 0.2, 0.0}, {0.8, 0.5, 0.2}, 150.};



constexpr Sphere spheres[] = {

    {{ 0,   -1.5, -15}, 1,      glass},       
    {{ 4,   2.0, -22}, 3,      mirror},       


};

constexpr vec3 lights[] = {
    {-20, 20,  20},
    { 30, 50, -25},
    { 30, 20,  30}
};

vec3 reflect(const vec3 &I, const vec3 &N) {
    return I - N*2.f*(I*N);
}

vec3 refract(const vec3 &I, const vec3 &N, const float eta_t, const float eta_i=1.f) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    if (cosi<0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? vec3{1,0,0} : I*eta + N*(eta*cosi - std::sqrt(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
}

std::vector<vec3> envmap;
int envmap_width, envmap_height;

void load_envmap(const char *filename) {
    int n = -1;
    unsigned char *pixmap = stbi_load(filename, &envmap_width, &envmap_height, &n, 0);
    if (!pixmap || n != 3) {
        std::cerr << "Error: unable to load the environment map" << std::endl;
        std::exit(1);
    }
    envmap = std::vector<vec3>(envmap_width * envmap_height);
    for (int j = 0; j < envmap_height; j++) {
        for (int i = 0; i < envmap_width; i++) {
            int idx = (i + j * envmap_width) * 3;
            envmap[i + (envmap_height - 1 - j) * envmap_width] = {
                pixmap[idx] / 255.f,
                pixmap[idx + 1] / 255.f,
                pixmap[idx + 2] / 255.f
            };
        }
    }
    stbi_image_free(pixmap);
}

std::tuple<bool,float> ray_sphere_intersect(const vec3 &orig, const vec3 &dir, const Sphere &s) { // ret value is a pair [intersection found, distance]
    vec3 L = s.center - orig;
    float tca = L*dir;
    float d2 = L*L - tca*tca;
    if (d2 > s.radius*s.radius) return {false, 0};
    float thc = std::sqrt(s.radius*s.radius - d2);
    float t0 = tca-thc, t1 = tca+thc;
    if (t0>.001) return {true, t0};  // offset the original point by .001 to avoid occlusion by the object itself
    if (t1>.001) return {true, t1};
    return {false, 0};
}

std::tuple<bool,vec3,vec3,Material> scene_intersect(const vec3 &orig, const vec3 &dir) {
    vec3 pt, N;
    Material material;

    float nearest_dist = 1e10;

    // Полукруг из сфер
    constexpr int num_spheres = 4; // Количество сфер
    constexpr float circle_radius = 5.0f; // Радиус полукруга
    constexpr float sphere_radius = 2.f; // Размер сфер
    constexpr Material materials[] = {ivory, gold, red_rubber, silver}; // Разные материалы

    for (int i = 0; i < num_spheres; ++i) {
        float angle = M_PI * i / (num_spheres - 1); // Угол для каждой сферы
        vec3 sphere_center = {circle_radius * cos(angle), -3, -15 - circle_radius * sin(angle)};
        Sphere temp_sphere = {sphere_center, sphere_radius, materials[i % 4]}; // Назначаем материал

        auto [intersection, d] = ray_sphere_intersect(orig, dir, temp_sphere);
        if (!intersection || d > nearest_dist) continue;

        nearest_dist = d;
        pt = orig + dir * nearest_dist;
        N = (pt - temp_sphere.center).normalized();
        material = temp_sphere.material;
    }

    // Пересечение с полом (шахматная структура)
    if (std::abs(dir.y) > .001) { // avoid division by zero
        float d = -(orig.y + 5.5) / dir.y; // the checkerboard plane has equation y = -4
        vec3 p = orig + dir * d;
        if (d > .001 && d < nearest_dist && std::abs(p.x) < 10 && p.z < -10 && p.z > -30) {
            nearest_dist = d;
            pt = p;
            N = {0, 1, 0};
            int cell_x = int(pt.x * 3) % 3;
            int cell_y = int(pt.y * 3) % 3;
            material.diffuse_color = (cell_x + cell_y) % 2 ? vec3{0.8, 0.7, 0.6} : vec3{0.2, 0.1, 0.1};
        }   
    }

    // Пересечение с вертикальной стеной
    if (std::abs(dir.x) > 0.001) { // avoid division by zero
        float d = -(orig.x + 7.3) / dir.x; // the wall plane has equation x = -10
        vec3 p = orig + dir * d;
        if (d > 0.001 && d < nearest_dist && p.y < 10 && p.y > -4 && p.z < -10 && p.z > -30) {
            nearest_dist = d;
            pt = p;
            N = {1, 0, 0};
            material.diffuse_color = (int(pt.x * 2 + pt.z * 2 + std::rand() % 2) % 2) ? vec3{0.5, 0.5, 0.5} : vec3{0.2, 0.2, 0.2};
        }
    }

    // Пересечение с существующими сферами
    for (const Sphere &s : spheres) {
        auto [intersection, d] = ray_sphere_intersect(orig, dir, s);
        if (!intersection || d > nearest_dist) continue;
        nearest_dist = d;
        pt = orig + dir * nearest_dist;
        N = (pt - s.center).normalized();
        material = s.material;
    }

    return {nearest_dist < 1000, pt, N, material};
}

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const int depth=0) {
    auto [hit, point, N, material] = scene_intersect(orig, dir);
    if (depth > 4 || !hit) {
        // Используем окружение как фон
        float phi = std::atan2(dir.z, dir.x);
        float theta = std::acos(dir.y);
        int x = static_cast<int>((phi + M_PI) / (2 * M_PI) * envmap_width);
        int y = static_cast<int>(theta / M_PI * envmap_height);
        return envmap[x + y * envmap_width];
    }

    vec3 reflect_dir = reflect(dir, N).normalized();
    vec3 refract_dir = refract(dir, N, material.refractive_index).normalized();
    vec3 reflect_color = cast_ray(point, reflect_dir, depth + 1);
    vec3 refract_color = cast_ray(point, refract_dir, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (const vec3 &light : lights) {
        vec3 light_dir = (light - point).normalized();
        auto [hit, shadow_pt, trashnrm, trashmat] = scene_intersect(point, light_dir);
        if (hit && (shadow_pt - point).norm() < (light - point).norm()) continue;
        diffuse_light_intensity  += std::max(0.f, light_dir * N);
        specular_light_intensity += std::pow(std::max(0.f, -reflect(-light_dir, N) * dir), material.specular_exponent);
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + 
           vec3{1., 1., 1.} * specular_light_intensity * material.albedo[1] + 
           reflect_color * material.albedo[2] + 
           refract_color * material.albedo[3];
}

int main() {
    constexpr int   width  = 1024;
    constexpr int   height = 768;
    constexpr float fov    = 1.05; // 60 degrees field of view in radians
    load_envmap("/home/snowwy/Desktop/MAI/computer_graphics/comp_graphics/lab5/assets/envmap.jpg"); // Загрузите вашу текстуру окружения
    unsigned char* pixel_data = new unsigned char[width * height * 3];
    
    int n = -1;
    unsigned char *pixmap = stbi_load("/home/snowwy/Desktop/MAI/computer_graphics/comp_graphics/lab5/assets/envmap.jpg", &envmap_width, &envmap_height, &n, 0);
    if (!pixmap || 3!=n) {
        std::cerr << "Error: can not load the environment map" << std::endl;
        return -1;
    }
    envmap = std::vector<vec3>(envmap_width * envmap_height);
        for (int j = 0; j < envmap_height; j++) {
        for (int i = 0; i < envmap_width; i++) {
            int idx = (i + (envmap_height -1 - j) * envmap_width) * 3;
            envmap[i + (envmap_height - 1 - j) * envmap_width] = {
                pixmap[idx] / 255.f,
                pixmap[idx + 1] / 255.f,
                pixmap[idx + 2] / 255.f
            };
        }
    }
    stbi_image_free(pixmap);
    
#pragma omp parallel for
    for (int pix = 0; pix < width * height; pix++) { // actual rendering loop
        float dir_x =  (pix % width + 0.5) -  width / 2.;
        float dir_y = -(pix / width + 0.5) + height / 2.; // this flips the image at the same time
        float dir_z = -height / (2. * tan(fov / 2.));
        const vec3 &color = cast_ray(vec3{0, 0, 0}, vec3{dir_x, dir_y, dir_z}.normalized());
        float max = std::max(1.f, std::max(color[0], std::max(color[1], color[2])));
        pixel_data[pix * 3]     = static_cast<unsigned char>(255 * color[0] / max);
        pixel_data[pix * 3 + 1] = static_cast<unsigned char>(255 * color[1] / max);
        pixel_data[pix * 3 + 2] = static_cast<unsigned char>(255 * color[2] / max);
    }


    // Получение текущего времени
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    // Формирование имени файла
    std::ostringstream filename;
    filename << "render_" 
             << (now->tm_year + 1900) << "-"
             << (now->tm_mon + 1) << "-"
             << now->tm_mday << "_"
             << now->tm_hour << "-"
             << now->tm_min << "-"
             << now->tm_sec << ".png";

    // Сохранение файла
    stbi_write_png(filename.str().c_str(), width, height, 3, pixel_data, width * 3);

    delete[] pixel_data;
    return 0;
}



