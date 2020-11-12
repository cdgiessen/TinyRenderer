#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
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

void triangle(vec3 *pts, std::vector<float> &zbuffer, TGAImage &image, TGAColor color)
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

vec3 world2screen(vec3 v)
{
    return vec3(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

void random_colors(Model &model, TGAImage &image)
{
    vec3 light_dir{0, 0, -1};
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        vec3 pts[3];
        for (int i = 0; i < 3; i++) pts[i] = world2screen(model.vert(face[i]));
        triangle(pts, zbuffer, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }
}

void lambert_lighting(vec3 light_dir, Model &model, TGAImage &image)
{
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (int i = 0; i < model.nfaces(); i++) {
        std::vector<int> face = model.face(i);
        vec3 screen_coords[3];
        vec3 world_coords[3];
        for (int i = 0; i < 3; i++) {
            screen_coords[i] = world2screen(model.vert(face[i]));
            world_coords[i] = model.vert(face[i]);
        }
        vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) {
            triangle(screen_coords, zbuffer, image,
                     TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
}

int main(int argc, char **argv)
{
    Model model{"obj/african_head.obj"};
    TGAImage image(width, height, TGAImage::RGB);
    vec3 light_dir{0, 0, -1};
    lambert_lighting(light_dir, model, image);
    // random_colors(model, image);
    image.write_tga_file("output.tga");
    return 0;
}