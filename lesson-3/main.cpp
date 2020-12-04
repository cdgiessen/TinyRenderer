#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
#include <array>
#include <algorithm>

const int width = 600;
const int height = 600;
using ZBuffer = std::vector<float>;

vec3f barycentric(vec3i A, vec3i B, vec3i C, vec3i P)
{
    vec3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = static_cast<double>(C[i] - A[i]);
        s[i][1] = static_cast<double>(B[i] - A[i]);
        s[i][2] = static_cast<double>(A[i] - P[i]);
    }
    vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) >
        1e-2)  // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return vec3f(-1, 1, 1);  // in this case generate negative coordinates, it will be thrown away
                             // by the rasterizer
}

void triangle(std::array<vec3i, 3> pts, std::vector<float> &zbuffer, TGAImage &image,
              TGAColor color)
{
    vec2i bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec2i bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
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

void triangle_textured(std::array<vec3i, 3> pts, std::array<vec2f, 3> uvs,
                       std::vector<float> &zbuffer, TGAImage &image, Model &model)
{
    vec2i bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec2i bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += static_cast<int>(pts[i][2] * bc_screen[i]);
            if (zbuffer[P.x + P.y * width] < P.z) {
                zbuffer[P.x + P.y * width] = P.z;
                vec2f uv{0, 0};
                for (int i = 0; i < 3; i++) {
                    uv.x += uvs[i].x * bc_screen[i];
                    uv.y += uvs[i].y * bc_screen[i];
                }
                TGAColor color = model.diffuse(uv);

                image.set(P.x, P.y, color);
            }
        }
    }
}

vec3i world2screen(vec3f v)
{
    return vec3i(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

void random_colors(Model &model, TGAImage &image)
{
    vec3f light_dir{0, 0, -1};
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec3i, 3> pts;
        for (int j = 0; j < 3; j++) pts[j] = world2screen(model.vert(i, j));
        triangle(pts, zbuffer, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }
}

void lambert_lighting(vec3f light_dir, Model &model, TGAImage &image)
{
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec3i, 3> screen_coords;
        std::array<vec3f, 3> world_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = world2screen(model.vert(i, j));
            world_coords[j] = model.vert(i, j);
        }
        vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
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

void lambert_textured_lighting(vec3f light_dir, Model &model, TGAImage &image)
{
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec3i, 3> screen_coords;
        std::array<vec3f, 3> world_coords;
        std::array<vec2f, 3> uv_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = world2screen(model.vert(i, j));
            world_coords[j] = model.vert(i, j);
            uv_coords[j] = model.uv(i, j);
        }
        vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = static_cast<float>(n * light_dir);

        if (intensity > 0) {
            triangle_textured(screen_coords, uv_coords, zbuffer, image, model);
        }
    }
    TGAImage depth(width, height, TGAImage::GRAYSCALE);
    for (size_t h = 0; h < height; h++)
        for (size_t w = 0; w < width; w++)
            depth.set(h, w, static_cast<uint8_t>((zbuffer[w * height + h] + 1) * 128));

    depth.write_tga_file("depth.tga");
}

int main()
{
    Model model{"obj/african_head.obj", true, false, false};
    TGAImage image(width, height, TGAImage::RGB);
    vec3f light_dir{0, 0, -1};
    // lambert_lighting(light_dir, model, image);
    // random_colors(model, image);
    lambert_textured_lighting(light_dir, model, image);
    image.write_tga_file("output.tga");
    return 0;
}