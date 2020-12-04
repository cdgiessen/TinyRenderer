#pragma once
#include "our_gl.h"

struct FlatShader : public IShader
{
    mat<3, 3> varying_tri;

    vec3f uniform_light_dir;

    virtual ~FlatShader() {}

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        vec4f gl_Vertex = embed<double, 4>(model.vert(iface, nthvert));
        gl_Vertex = Projection * ModelView * gl_Vertex;
        varying_tri.set_col(nthvert, proj<double, 3>(gl_Vertex / gl_Vertex[3]));
        gl_Vertex = Viewport * gl_Vertex;
        return proj<double, 4>(gl_Vertex / gl_Vertex[3]);
    }

    virtual bool fragment(vec3f bar, TGAColor& color)
    {
        vec3f n =
            cross(varying_tri.col(1) - varying_tri.col(0), varying_tri.col(2) - varying_tri.col(0))
                .normalize();
        double intensity = clamp(n * uniform_light_dir, 0., 1.);
        color = TGAColor(255, 255, 255) * intensity;
        return false;
    }
};

struct GouraudShader : public IShader
{
    mat<3, 3> varying_tri;
    vec3f varying_ity;

    vec3f uniform_light_dir;

    virtual ~GouraudShader() {}

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        vec4f gl_Vertex = embed<double, 4>(model.vert(iface, nthvert));
        gl_Vertex = Projection * ModelView * gl_Vertex;
        varying_tri.set_col(nthvert, proj<double, 3>(gl_Vertex / gl_Vertex[3]));

        varying_ity[nthvert] = clamp(model.normal(iface, nthvert) * uniform_light_dir, 0., 1.);

        gl_Vertex = Viewport * gl_Vertex;
        return proj<double, 4>(gl_Vertex / gl_Vertex[3]);
    }

    virtual bool fragment(vec3f bar, TGAColor& color)
    {
        double intensity = varying_ity * bar;
        color = TGAColor(255, 255, 255) * intensity;
        return false;
    }
};

struct ToonShader : public IShader
{
    mat<3, 3> varying_tri;
    vec3f varying_ity;

    vec3f uniform_light_dir;

    virtual ~ToonShader() {}

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        vec4f gl_Vertex = embed<double, 4>(model.vert(iface, nthvert));
        gl_Vertex = Projection * ModelView * gl_Vertex;
        varying_tri.set_col(nthvert, proj<double, 3>(gl_Vertex / gl_Vertex[3]));

        varying_ity[nthvert] = clamp(model.normal(iface, nthvert) * uniform_light_dir, 0., 1.);

        gl_Vertex = Viewport * gl_Vertex;
        return proj<double, 4>(gl_Vertex / gl_Vertex[3]);
    }

    virtual bool fragment(vec3f bar, TGAColor& color)
    {
        double intensity = varying_ity * bar;
        if (intensity > .85)
            intensity = 1;
        else if (intensity > .60)
            intensity = .80;
        else if (intensity > .45)
            intensity = .60;
        else if (intensity > .30)
            intensity = .45;
        else if (intensity > .15)
            intensity = .30;
        color = TGAColor(255, 155, 0) * intensity;
        return false;
    }
};