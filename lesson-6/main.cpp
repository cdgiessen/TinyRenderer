#include "our_gl.h"

#include "tgaimage.h"
#include "model.h"

#include "shaders.h"

const int width = 800;
const int height = 800;

vec3f light_dir(1, 1, 1);
vec3f eye(1, 1, 3);
vec3f center(0, 0, 0);
vec3f up(0, 1, 0);

int main()
{
    Model model{"obj/african_head.obj", true, false, false};

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1. / (eye - center).norm());
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    GouraudShader shader;
    shader.uniform_light_dir = light_dir;
    for (size_t i = 0; i < model.nfaces(); i++) {
        std::array<vec4f, 3> screen_coords;
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = shader.vertex(model, i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    // image.flip_vertically();  // to place the origin in the bottom left corner of the image
    // zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    return 0;
}