#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct TGA_Header
{
    std::uint8_t idlength{};
    std::uint8_t colormaptype{};
    std::uint8_t datatypecode{};
    std::uint16_t colormaporigin{};
    std::uint16_t colormaplength{};
    std::uint8_t colormapdepth{};
    std::uint16_t x_origin{};
    std::uint16_t y_origin{};
    std::uint16_t width{};
    std::uint16_t height{};
    std::uint8_t bitsperpixel{};
    std::uint8_t imagedescriptor{};
};
#pragma pack(pop)
struct TGAColor
{
    std::uint8_t bgra[4] = {0, 0, 0, 0};
    std::uint8_t bytespp = {0};

    TGAColor() = default;
    TGAColor(const std::uint8_t R, const std::uint8_t G, const std::uint8_t B,
             const std::uint8_t A = 255)
        : bgra{B, G, R, A}, bytespp(4)
    {}
    TGAColor(const std::uint8_t v) : bgra{v, 0, 0, 0}, bytespp(1) {}

    TGAColor(const std::uint8_t *p, const std::uint8_t bpp) : bgra{0, 0, 0, 0}, bytespp(bpp)
    {
        for (size_t i = 0; i < bpp; i++) bgra[i] = p[i];
    }

    std::uint8_t &operator[](const size_t i) { return bgra[i]; }

    TGAColor operator*(const double intensity) const
    {
        TGAColor res = *this;
        double clamped = std::max(0., std::min(intensity, 1.));
        for (size_t i = 0; i < 4; i++) res.bgra[i] = static_cast<uint8_t>(bgra[i] * clamped);
        return res;
    }
};

class TGAImage
{
protected:
    std::vector<std::uint8_t> data;
    uint32_t width{};
    uint32_t height{};
    uint32_t bytespp{};

    bool load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out) const;

public:
    enum Format
    {
        GRAYSCALE = 1,
        RGB = 3,
        RGBA = 4
    };

    TGAImage();
    TGAImage(const size_t w, const size_t h, const size_t bpp);
    bool read_tga_file(const std::string filename);
    bool write_tga_file(const std::string filename, const bool v_flip = true,
                        const bool rle = true) const;
    void flip_horizontally();
    void flip_vertically();
    void scale(const size_t w, const size_t h);
    TGAColor get(const size_t x, const size_t y) const;
    void set(const size_t x, const size_t y, const TGAColor &c);
    size_t get_width() const;
    size_t get_height() const;
    size_t get_bytespp();
    std::uint8_t *buffer();
    void clear();
};
