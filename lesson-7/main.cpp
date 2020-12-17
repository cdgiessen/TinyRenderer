#include "our_gl.h"

#include "tgaimage.h"
#include "model.h"

const int width = 1000;
const int height = 1000;

vec3f light_dir(1, 1, 1);
vec3f eye(1, 1, 3);
vec3f center(0, 0, 0);
vec3f up(0, 1, 0);

struct DepthShader : public IShader
{
    mat<3, 3> varying_tri;

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        vec4f gl_Vertex = embed<4>(model.vert(iface, nthvert));  // read the vertex from .obj file
        gl_Vertex = uniform_Viewport * uniform_Projection * uniform_ModelView *
                    gl_Vertex;  // transform it to screen coordinates
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Model& model, vec3f bar, TGAColor& color)
    {
        vec3f p = varying_tri * bar;
        color = TGAColor(255, 255, 255) * (p.z / 500.f);
        return false;
    }
};
struct Shader : public IShader
{
    mat<4, 4> uniform_M;        //  Projection*ModelView
    mat<4, 4> uniform_MIT;      // (Projection*ModelView).invert_transpose()
    mat<4, 4> uniform_Mshadow;  // transform framebuffer screen coordinates to shadowbuffer screen
                                // coordinates
    mat<2, 3> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the
                           // fragment shader
    mat<3, 3>
        varying_tri;  // triangle coordinates before Viewport transform, written by VS, read by FS
    DepthBuffer& shadow_buffer;  // shadow_buffer

    Shader(mat4 M, mat4 MIT, mat4 MS, DepthBuffer& shadow_buffer)
        : uniform_M(M),
          uniform_MIT(MIT),
          uniform_Mshadow(MS),
          varying_uv(),
          shadow_buffer(shadow_buffer),
          varying_tri()
    {}

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model.uv(iface, nthvert));
        vec4f gl_Vertex = uniform_Viewport * uniform_Projection * uniform_ModelView *
                          embed<4>(model.vert(iface, nthvert));
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Model& model, vec3f bar, TGAColor& color)
    {
        vec4f sb_p = uniform_Mshadow *
                     embed<4>(varying_tri * bar);  // corresponding point in the shadow buffer
        sb_p = sb_p / sb_p[3];
        double shadow = .3 + .7 * (shadow_buffer.get(int(sb_p[0]), int(sb_p[1])) <=
                                   sb_p[2] + 43.34);  // magic coeff to avoid z-fighting
        // shadow = std::max(0., shadow);

        // shadow = .7 * shadow_buffer.get(int(sb_p[0]), int(sb_p[1]));
        vec2f uv = varying_uv * bar;  // interpolate uv for the current pixel
        vec3f n =
            proj<3>(uniform_MIT * embed<4>(model.normal(uv).normalize())).normalize();  // normal
        vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();  // light vector
        vec3f r = (n * (dot(n, l) * 2.) - l).normalize();                // reflected light
        double spec = std::pow(std::max(r.z, 0.0), model.specular(uv));
        double diff = std::max(0., dot(n, l));
        TGAColor c = model.diffuse(uv);
        for (int i = 0; i < 3; i++) {
            color[i] =
                static_cast<uint8_t>(std::min(20 + c[i] * shadow * (1.2 * diff + .6 * spec), 255.));
            // color[i] = (uint8_t)(model.specular(uv) + 64);
        }

        return false;
    }
};

int main()
{
    Model model{"obj/african_head.obj", true, true, true};

    light_dir = light_dir.normalize();

    mat4 ModelView = lookat(light_dir, center, up);
    mat4 Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    mat4 Projection = projection(0);

    mat4 M = Viewport * Projection * ModelView;

    TGAImage image(width, height, TGAImage::RGB);
    DepthBuffer zbuffer(width, height);
    DepthBuffer shadow_buffer(width, height);

    {  // rendering the shadow buffer
        TGAImage shadow_texture(width, height, TGAImage::RGB);
        DepthShader depth_shader;
        depth_shader.uniform_ModelView = ModelView;
        depth_shader.uniform_Viewport = Viewport;
        depth_shader.uniform_Projection = Projection;
        std::array<vec4f, 3> screen_coords;
        for (size_t i = 0; i < model.nfaces(); i++) {
            for (size_t j = 0; j < 3; j++) {
                screen_coords[j] = depth_shader.vertex(model, i, j);
            }
            triangle(model, screen_coords, depth_shader, shadow_texture, shadow_buffer);
        }
        shadow_texture.write_tga_file("depth.tga");
    }

    ModelView = lookat(eye, center, up);
    Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    Projection = projection(-1. / (eye - center).norm());

    Shader shader{ModelView, (Projection * ModelView).invert_transpose(),
                  M * (Viewport * Projection * ModelView).invert(), shadow_buffer};
    shader.uniform_ModelView = ModelView;
    shader.uniform_Viewport = Viewport;
    shader.uniform_Projection = Projection;
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec4f, 3> screen_coords;
        for (size_t j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(model, i, j);
        }
        triangle(model, screen_coords, shader, image, zbuffer);
    }

    image.write_tga_file("output.tga");
    zbuffer.write("zbuffer.tga");

    return 0;
}