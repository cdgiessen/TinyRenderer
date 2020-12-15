#include "our_gl.h"

#include <cmath>
#include <limits>
#include <cstdlib>

IShader::~IShader() {}

DepthBuffer::DepthBuffer(size_t width, size_t height)
    : width(width), height(height), data(width * height, -std::numeric_limits<double>::max())
{}
double DepthBuffer::get(size_t x, size_t y) const { return data[x * width + y]; }
void DepthBuffer::set(size_t x, size_t y, double value) { data[x * width + y] = value; }

void DepthBuffer::write(const char *filename) const
{
    TGAImage image(width, height, TGAImage::GRAYSCALE);
    for (size_t i = 0; i < width; i++) {
        for (size_t j = 0; j < height; j++) {
            image.set(i, j, TGAColor(static_cast<uint8_t>(data[i * width + j])));
        }
    }
    image.write_tga_file(filename);
}
mat4 viewport(int x, int y, int w, int h)
{
    mat4 Viewport = mat4::identity();
    Viewport[0][3] = x + w / 2.f;
    Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] = 255.f / 2.f;
    Viewport[0][0] = w / 2.f;
    Viewport[1][1] = h / 2.f;
    Viewport[2][2] = 255.f / 2.f;
    return Viewport;
}

mat4 projection(double coeff)
{
    mat4 Projection = mat4::identity();
    Projection[3][2] = coeff;
    return Projection;
}

mat4 lookat(vec3f eye, vec3f center, vec3f up)
{
    vec3f z = (eye - center).normalize();
    vec3f x = cross(up, z).normalize();
    vec3f y = cross(z, x).normalize();
    mat4 ModelView = mat4::identity();
    for (int i = 0; i < 3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
    return ModelView;
}

vec3f barycentric(vec2f A, vec2f B, vec2f C, vec2f P)
{
    vec3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)  // dont forget that u[2] is integer. If it is zero then triangle
                                // ABC is degenerate
        return vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return vec3f(-1, 1, 1);  // in this case generate negative coordinates, it will be thrown
                             // away by the rasterizer
}

void triangle(Model &model, std::array<vec4f, 3> pts, IShader &shader, TGAImage &image,
              DepthBuffer &zbuffer)
{
    vec2f bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    vec2f bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    vec2i P;
    TGAColor color;
    for (P.x = static_cast<int>(bboxmin.x); P.x <= static_cast<int>(bboxmax.x); P.x++) {
        for (P.y = static_cast<int>(bboxmin.y); P.y <= static_cast<int>(bboxmax.y); P.y++) {
            vec3f c = barycentric(proj<2, 4>(pts[0] / pts[0][3]), proj<2, 4>(pts[1] / pts[1][3]),
                                  proj<2, 4>(pts[2] / pts[2][3]), to_f(proj<2, 2>(P)));
            double z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            double w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            double frag_depth = std::max(0., std::min(255., z / w + .5));
            if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y) > frag_depth) continue;
            bool discard = shader.fragment(model, c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, frag_depth);
                image.set(P.x, P.y, color);
            }
        }
    }
}
