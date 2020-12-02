#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
#include <array>
#include <algorithm>

const int width = 600;
const int height = 600;
using ZBuffer = std::vector<float>;

vec3 barycentric(vec3 A, vec3 B, vec3 C, vec3 P)
{
    vec3 s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    vec3 u = cross(s[0], s[1]);
    if (std::abs(u[2]) >
        1e-2)  // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return vec3(-1, 1, 1);  // in this case generate negative coordinates, it will be thrown away by
                            // the rasterizer
}

void triangle(std::array<vec3, 3> pts, std::vector<float> &zbuffer, TGAImage &image, TGAColor color)
{
    vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec2 bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    vec2 clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0., std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vec3 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            vec3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = static_cast<float>(P.z);
                image.set(static_cast<int>(P.x), static_cast<int>(P.y), color);
            }
        }
    }
}

void triangle_textured(std::array<vec3, 3> pts, std::array<vec2, 3> uvs,
                       std::vector<float> &zbuffer, TGAImage &image, Model &model)
{
    vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec2 bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    vec2 clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0., std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vec3 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            vec3 bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = static_cast<float>(P.z);
                vec2 uv{0, 0};
                for (int i = 0; i < 3; i++) {
                    uv.x += uvs[i].x * bc_screen[i];
                    uv.y += uvs[i].y * bc_screen[i];
                }
                TGAColor color = model.diffuse(uv);

                image.set(static_cast<int>(P.x), static_cast<int>(P.y), color);
            }
        }
    }
}

vec3 world2screen(vec3 v)
{
    return vec3(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

void random_colors(Model &model, TGAImage &image)
{
    vec3 light_dir{0, 0, -1};
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (int i = 0; i < model.nfaces(); i++) {
        std::array<vec3, 3> pts;
        for (int j = 0; j < 3; j++) pts[j] = world2screen(model.vert(i, j));
        triangle(pts, zbuffer, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }
}

void lambert_lighting(vec3 light_dir, Model &model, TGAImage &image)
{
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (int i = 0; i < model.nfaces(); i++) {
        std::array<vec3, 3> screen_coords;
        std::array<vec3, 3> world_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = world2screen(model.vert(i, j));
            world_coords[j] = model.vert(i, j);
        }
        vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        double intensity = n * light_dir;
        if (intensity > 0) {
            triangle(screen_coords, zbuffer, image,
                     TGAColor(static_cast<uint8_t>(intensity * 255),
                              static_cast<uint8_t>(intensity * 255),
                              static_cast<uint8_t>(intensity * 255), 255));
        }
    }
}

void lambert_textured_lighting(vec3 light_dir, Model &model, TGAImage &image)
{
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (int i = 0; i < model.nfaces(); i++) {
        std::array<vec3, 3> screen_coords;
        std::array<vec3, 3> world_coords;
        std::array<vec2, 3> uv_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = world2screen(model.vert(i, j));
            world_coords[j] = model.vert(i, j);
            uv_coords[j] = model.uv(i, j);
        }
        vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;

        if (intensity > 0) {
            triangle_textured(screen_coords, uv_coords, zbuffer, image, model);
        }
    }
    TGAImage depth(width, height, TGAImage::GRAYSCALE);
    for (int h = 0; h < height; h++)
        for (int w = 0; w < width; w++) depth.set(h, w, (zbuffer[w * height + h] + 1) * 128);

    depth.write_tga_file("depth.tga");
}

int main(int argc, char **argv)
{
    Model model{"obj/african_head.obj", true, false, false};
    TGAImage image(width, height, TGAImage::RGB);
    vec3 light_dir{0, 0, -1};
    // lambert_lighting(light_dir, model, image);
    // random_colors(model, image);
    lambert_textured_lighting(light_dir, model, image);
    image.write_tga_file("output.tga");
    return 0;
}