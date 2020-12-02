#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
#include <array>
#include <algorithm>

const int width = 600;
const int height = 600;
const int depth = 255;
using ZBuffer = std::vector<float>;

vec3 light_dir = vec3(1, 1, 1).normalize();
vec3 eye(0.5, 0.2, 4);
vec3 center(0, 0, 0);
using mat4 = mat<4, 4>;

mat4 viewport(int x, int y, int w, int h)
{
    mat4 m = mat4::identity();
    m[0][3] = x + w / 2.;
    m[1][3] = y + h / 2.;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.;
    m[1][1] = h / 2.;
    m[2][2] = depth / 2.f;
    return m;
}

mat4 projection()
{
    mat4 matrix = mat4::identity();
    matrix[3][2] = -1.f / (eye - center).norm();
    return matrix;
}

mat4 lookat(vec3 eye, vec3 center, vec3 up)
{
    vec3 z = (eye - center).normalize();
    vec3 x = cross(up, z).normalize();
    vec3 y = cross(z, x).normalize();
    mat4 res = mat4::identity();
    for (size_t i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

vec3 barycentric(vec3 A, vec3 B, vec3 C, vec3 P)
{
    vec3 u = cross(vec3(C.x - A.x, B.x - A.x, A.x - P.x), vec3(C.y - A.y, B.y - A.y, A.y - P.y));
    return std::abs(u.z) > .5 ? vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z)
                              : vec3(-1, 1, 1);  // dont forget that u.z is an integer. If it is
                                                 // zero then triangle ABC is degenerate
}

void triangle_textured(std::array<vec3, 3> pts, std::array<vec2, 3> uvs,
                       std::vector<float> &zbuffer, TGAImage &image, Model &model)
{
    vec2 bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec2 bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    vec3 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x = int(P.x + 1)) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y = int(P.y + 1)) {
            vec3 bc = barycentric(pts[0], pts[1], pts[2], P);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            P.z = std::max(
                0, std::min(255, int(pts[0].z * bc.x + pts[1].z * bc.y + pts[2].z * bc.z + .5)));
            if (zbuffer[size_t(P.x) * width + size_t(P.y)] < P.z) {
                zbuffer[size_t(P.x) * width + size_t(P.y)] = static_cast<float>(P.z);
                vec2 uv{0, 0};
                for (size_t i = 0; i < 3; i++) {
                    uv.x += uvs[i].x * bc[i];
                    uv.y += uvs[i].y * bc[i];
                }
                TGAColor color = model.diffuse(uv);

                image.set(static_cast<size_t>(P.x), static_cast<size_t>(P.y), color);
            }
        }
    }
}

void lambert_textured_lighting(Model &model, TGAImage &image)
{
    mat4 model_view = lookat(eye, center, vec3(0, 1, 0));
    mat4 proj = projection();
    mat4 view_port = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec3, 3> screen_coords;
        std::array<vec3, 3> world_coords;
        std::array<vec2, 3> uv_coords;

        vec3 n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
        n.normalize();
        for (size_t j = 0; j < 3; j++) {
            screen_coords[j] = embed<3>(view_port * proj * model_view * embed<4>(model.vert(i, j)));
            world_coords[j] = model.vert(i, j);
            uv_coords[j] = model.uv(i, j);
        }

        triangle_textured(screen_coords, uv_coords, zbuffer, image, model);
    }
    TGAImage depth(width, height, TGAImage::GRAYSCALE);
    for (size_t w = 0; w < width; w++)
        for (size_t h = 0; h < height; h++)
            depth.set(h, w, static_cast<uint8_t>((zbuffer[h * height + w] + 1.f)));

    depth.write_tga_file("depth.tga");
}

int main()
{
    Model model{"obj/african_head.obj", true, false, false};
    TGAImage image(width, height, TGAImage::RGB);
    lambert_textured_lighting(model, image);
    image.write_tga_file("output.tga");
    return 0;
}