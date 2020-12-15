#pragma once
#include <vector>
#include <array>
#include <algorithm>

#include "geometry.h"
#include "model.h"

mat4 viewport(int x, int y, int w, int h);
mat4 projection(double coeff = 0.f);  // coeff = -1/c
mat4 lookat(vec3f eye, vec3f center, vec3f up);

struct IShader
{
    mat4 uniform_ModelView;
    mat4 uniform_Viewport;
    mat4 uniform_Projection;

    virtual ~IShader();
    virtual vec4f vertex(Model &model, int iface, int nthvert) = 0;
    virtual bool fragment(Model &model, vec3f bar, TGAColor &color) = 0;
};

struct DepthBuffer
{
private:
    size_t width = 0, height = 0;
    std::vector<double> data;

public:
    DepthBuffer(size_t width, size_t height);
    double get(size_t x, size_t y) const;
    void set(size_t x, size_t y, double value);
    void write(const char *filename = "zbuffer.tga") const;
};

void triangle(Model &model, std::array<vec4f, 3> pts, IShader &shader, TGAImage &image,
              DepthBuffer &zbuffer);