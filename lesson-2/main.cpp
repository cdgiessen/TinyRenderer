#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <algorithm>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const int width = 1200;
const int height = 1200;

vec3f barycentric(vec2i *pts, vec2i P)
{
    vec3f u = cross(vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]),
                    vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
    /* `pts` and `P` has integer value as coordinates
       so `abs(u[2])` < 1 means `u[2]` is 0, that means triangle is degenerate
       in this case return something with negative coordinates */
    if (std::abs(u[2]) < 1) return vec3f(-1, 1, 1);
    return vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void triangle(vec2i *pts, TGAImage &image, TGAColor color)
{
    vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    vec2i bboxmax(0, 0);
    vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            image.set(static_cast<int>(P.x), static_cast<int>(P.y), color);
        }
    }
}

void random_colors(Model &model, TGAImage &image)
{
    for (size_t i = 0; i < model.nfaces(); i++) {
        vec2i screen_coords[3];
        for (size_t j = 0; j < 3; j++) {
            vec3f world_coords = model.vert(i, j);
            screen_coords[j] =
                vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
        }
        triangle(screen_coords, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }
}

void lambert_lighting(vec3f light_dir, Model &model, TGAImage &image)
{
    for (size_t i = 0; i < model.nfaces(); i++) {
        vec2i screen_coords[3];
        vec3f world_coords[3];
        for (size_t j = 0; j < 3; j++) {
            vec3f v = model.vert(i, j);
            screen_coords[j] = vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            world_coords[j] = v;
        }
        vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        double intensity = n * light_dir;
        if (intensity > 0) {
            triangle(screen_coords, image,
                     TGAColor(static_cast<uint8_t>(intensity * 255),
                              static_cast<uint8_t>(intensity * 255),
                              static_cast<uint8_t>(intensity * 255), 255));
        }
    }
}

int main()
{
    Model model{"obj/african_head.obj"};
    TGAImage image(width, height, TGAImage::RGB);
    vec3f light_dir{0, 0, -1};
    lambert_lighting(light_dir, model, image);

    image.write_tga_file("output.tga");
    return 0;
}