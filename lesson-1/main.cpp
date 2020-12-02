#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line_attempt_1(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    for (float t = 0.f; t < 1.f; t += .01f) {
        int x = static_cast<int>(static_cast<float>(x0) + static_cast<float>(x1 - x0) * t);
        int y = static_cast<int>(static_cast<float>(y0) + static_cast<float>(y1 - y0) * t);
        image.set(x, y, color);
    }
}

void line_attempt_2(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    for (int x = x0; x <= x1; x++) {
        float t = static_cast<float>(x - x0) / static_cast<float>(x1 - x0);
        int y = static_cast<int>(y0 * (1. - t) + static_cast<float>(y1) * t);
        image.set(x, y, color);
    }
}

void line_attempt_3(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {  // if the line is steep, we transpose the image
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {  // make it left−to−right
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (int x = x0; x <= x1; x++) {
        float t = static_cast<float>(x - x0) / static_cast<float>(x1 - x0);
        int y = static_cast<int>(y0 * (1. - t) + static_cast<float>(y1) * t);
        if (steep) {
            image.set(y, x, color);  // if transposed, de−transpose
        } else {
            image.set(x, y, color);
        }
    }
}
void line_attempt_4(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    float derror = std::abs(static_cast<float>(dy) / static_cast<float>(dx));
    float error = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error += derror;
        if (error > .5f) {
            y += (y1 > y0 ? 1 : -1);
            error -= 1.f;
        }
    }
}

void line_attempt_5(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}
int main()
{
    TGAImage image(300, 300, TGAImage::RGB);

    line_attempt_5(-100, -100, 500, 500, image, white);
    line_attempt_5(40, 40, 60, 260, image, white);
    line_attempt_5(40, 40, 260, 60, image, white);

    image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}