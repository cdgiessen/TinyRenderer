#pragma once
#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model
{
private:
    std::vector<vec3f> verts_;  // array of vertices
    std::vector<vec2f> uv_;     // array of tex coords
    std::vector<vec3f> norms_;  // array of normal vectors
    std::vector<int> facet_vrt_;
    std::vector<int> facet_tex_;  // indices in the above arrays per triangle
    std::vector<int> facet_nrm_;
    TGAImage diffusemap_;   // diffuse color texture
    TGAImage normalmap_;    // normal map texture
    TGAImage specularmap_;  // specular map texture
    void load_texture(const std::string filename, const std::string suffix, TGAImage &img);

public:
    Model(const std::string filename, bool diffuse_texture = false, bool normal_map = false,
          bool specular_texture = false);
    size_t nverts() const;
    size_t nfaces() const;
    vec3f normal(const size_t iface,
                 const size_t nthvert) const;  // per triangle corner normal vertex
    vec3f normal(const vec2f &uv) const;  // fetch the normal vector from the normal map texture
    vec3f vert(const size_t i) const;
    vec3f vert(const size_t iface, const size_t nthvert) const;
    vec2f uv(const size_t iface, const size_t nthvert) const;
    TGAColor diffuse(const vec2f &uv) const;
    double specular(const vec2f &uv) const;
};