#include "our_gl.h"

#include "tgaimage.h"
#include "model.h"

const int width = 800;
const int height = 800;

vec3f light_dir(-1, -1, 0);
vec3f eye(1, 1, 3);
vec3f center(0, 0, 0);
vec3f up(0, 1, 0);

struct PhongShader : public IShader
{
    mat<2, 3> varying_uv;   // triangle uv coordinates, written by the vertex shader, read by the
                            // fragment shader
    mat<4, 3> varying_tri;  // triangle coordinates (clip coordinates), written by VS, read by FS
    mat<3, 3> varying_nrm;  // normal per vertex to be interpolated by FS
    mat<3, 3> ndc_tri;      // triangle in normalized device coordinates

    vec3f uniform_light_dir;

    virtual vec4f vertex(Model& model, int iface, int nthvert)
    {
        vec4f gl_Vertex =
            uniform_Projection * uniform_ModelView * embed<4>(model.vert(iface, nthvert));
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

        varying_tri.set_col(nthvert, gl_Vertex);
        varying_uv.set_col(nthvert, model.uv(iface, nthvert));
        varying_nrm.set_col(nthvert,
                            proj<3>((uniform_Projection * uniform_ModelView).invert_transpose() *
                                    embed<4>(model.normal(iface, nthvert), 0.)));
        return uniform_Viewport * gl_Vertex;
    }

    virtual bool fragment(Model& model, vec3f bar, TGAColor& color)
    {
        vec3f bn = (varying_nrm * bar).normalize();
        vec2f uv = varying_uv * bar;

        mat<3, 3> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;

        mat<3, 3> AI = A.invert();

        vec3f i =
            AI * vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        vec3f j =
            AI * vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        mat<3, 3> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        vec3f n = (B * model.normal(uv)).normalize();

        double diff = std::max(0., dot(n, light_dir));
        color = model.diffuse(uv) * diff;

        return false;
    }
};

int main()
{
    Model model{"obj/african_head.obj", true, false, false};

    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    DepthBuffer zbuffer(width, height);

    // GouraudShader shader;
    PhongShader shader;
    shader.uniform_ModelView = lookat(eye, center, up);
    shader.uniform_Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    shader.uniform_Projection = projection(-1. / (eye - center).norm());
    shader.uniform_light_dir = light_dir;
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec4f, 3> screen_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(model, i, j);
        }
        triangle(model, screen_coords, shader, image, zbuffer);
    }

    // image.flip_vertically();  // to place the origin in the bottom left corner of the image
    // zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write("zbuffer.tga");

    return 0;
}