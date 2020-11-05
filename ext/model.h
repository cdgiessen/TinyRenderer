#pragma once
#include <vector>
#include "geometry.h"

class Model
{
private:
    std::vector<vec3> verts_;
    std::vector<std::vector<int> > faces_;

public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    vec3 vert(int i);
    std::vector<int> face(int idx);
};
