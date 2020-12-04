#pragma once
#include <vector>
#include <array>
#include <algorithm>

#include "geometry.h"
#include "model.h"

extern mat4 ModelView;
extern mat4 Viewport;
extern mat4 Projection;

void viewport(int x, int y, int w, int h);
void projection(double coeff = 0.f);  // coeff = -1/c
void lookat(vec3f eye, vec3f center, vec3f up);

struct IShader
{
    virtual ~IShader();
    virtual vec4f vertex(Model &model, int iface, int nthvert) = 0;
    virtual bool fragment(vec3f bar, TGAColor &color) = 0;
};

void triangle(std::array<vec4f, 3> pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);