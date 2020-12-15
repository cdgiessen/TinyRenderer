#pragma once

#include <cmath>
#include <cassert>
#include <iostream>
#include <fstream>

template <size_t n, typename T = double>
struct vec
{
    vec() = default;
    T& operator[](const size_t i)
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
    T operator[](const size_t i) const
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
    T norm2() const { return (*this) * (*this); }
    T norm() const { return std::sqrt(norm2()); }
    T data[n] = {0};
};

template <size_t n, typename T = double>
double dot(const vec<n, T>& lhs, const vec<n, T>& rhs)
{
    double ret = 0;
    for (size_t i = n; i--; ret += lhs[i] * rhs[i])
        ;
    return ret;
}

template <size_t n, typename T = double>
vec<n, T> operator+(const vec<n, T>& lhs, const vec<n, T>& rhs)
{
    vec<n, T> ret = lhs;
    for (size_t i = n; i--; ret[i] += rhs[i])
        ;
    return ret;
}

template <size_t n, typename T = double>
vec<n, T> operator-(const vec<n, T>& lhs, const vec<n, T>& rhs)
{
    vec<n, T> ret = lhs;
    for (size_t i = n; i--; ret[i] -= rhs[i])
        ;
    return ret;
}

template <size_t n, typename T = double>
vec<n, T> operator*(const double& rhs, const vec<n, T>& lhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] *= rhs)
        ;
    return ret;
}

template <size_t n, typename T = double>
vec<n, T> operator*(const vec<n, T>& lhs, const double& rhs)
{
    vec<n, T> ret = lhs;
    for (size_t i = n; i--; ret[i] *= rhs)
        ;
    return ret;
}

template <size_t n, typename T = double>
vec<n, T> operator/(const vec<n, T>& lhs, const double& rhs)
{
    vec<n, T> ret = lhs;
    for (size_t i = n; i--; ret[i] /= rhs)
        ;
    return ret;
}

template <size_t n1, size_t n2, typename T = double>
vec<n1, T> embed(const vec<n2, T>& v, double fill = 1)
{
    vec<n1, T> ret;
    for (size_t i = n1; i--; ret[i] = (i < n2 ? v[i] : fill))
        ;
    return ret;
}

template <size_t n1, size_t n2, typename T = double>
vec<n1, T> proj(const vec<n2, T>& v)
{
    vec<n1, T> ret;
    for (size_t i = n1; i--; ret[i] = v[i])
        ;
    return ret;
}

template <size_t n, typename T = double>
std::ostream& operator<<(std::ostream& out, const vec<n, T>& v)
{
    for (size_t i = 0; i < n; i++) out << v[i] << " ";
    return out;
}

template <typename T>
struct vec<2, T>
{
    vec() = default;
    vec(T X, T Y) : x(X), y(Y) {}
    T& operator[](const size_t i)
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }
    T operator[](const size_t i) const
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }
    T norm2() const { 
        return x * x + y * y; }
    T norm() const { return std::sqrt(norm2()); }
    vec& normalize()
    {
        *this = (*this) / norm();
        return *this;
    }

    T x{}, y{};
};

template <typename T>
struct vec<3, T>
{
    vec() = default;
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    T& operator[](const size_t i)
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (1 == i ? y : z);
    }
    T operator[](const size_t i) const
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (1 == i ? y : z);
    }
    T norm2() const { return dot((*this), (*this)); }
    T norm() const { return std::sqrt(norm2()); }
    vec& normalize()
    {
        *this = (*this) / norm();
        return *this;
    }

    T x{}, y{}, z{};
};

template <size_t n>
struct dt;

template <size_t nrows, size_t ncols>
struct mat
{
    vec<ncols, double> rows[nrows] = {{}};

    mat() = default;
    vec<ncols, double>& operator[](const size_t idx)
    {
        assert(idx >= 0 && idx < nrows);
        return rows[idx];
    }
    const vec<ncols, double>& operator[](const size_t idx) const
    {
        assert(idx >= 0 && idx < nrows);
        return rows[idx];
    }

    vec<nrows, double> col(const size_t idx) const
    {
        assert(idx >= 0 && idx < ncols);
        vec<nrows, double> ret;
        for (size_t i = nrows; i--; ret[i] = rows[i][idx])
            ;
        return ret;
    }

    void set_col(const size_t idx, const vec<nrows, double>& v)
    {
        assert(idx >= 0 && idx < ncols);
        for (size_t i = nrows; i--; rows[i][idx] = v[i])
            ;
    }

    static mat<nrows, ncols> identity()
    {
        mat<nrows, ncols> ret;
        for (size_t i = nrows; i--;)
            for (size_t j = ncols; j--; ret[i][j] = (i == j))
                ;
        return ret;
    }

    double det() const { return dt<ncols>::det(*this); }

    mat<nrows - 1, ncols - 1> get_minor(const size_t row, const size_t col) const
    {
        mat<nrows - 1, ncols - 1> ret;
        for (size_t i = nrows - 1; i--;)
            for (size_t j = ncols - 1; j--;
                 ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1])
                ;
        return ret;
    }

    double cofactor(const size_t row, const size_t col) const
    {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }

    mat<nrows, ncols> adjugate() const
    {
        mat<nrows, ncols> ret;
        for (size_t i = nrows; i--;)
            for (size_t j = ncols; j--; ret[i][j] = cofactor(i, j))
                ;
        return ret;
    }

    mat<nrows, ncols> invert_transpose() const
    {
        mat<nrows, ncols> ret = adjugate();
        return ret / dot(ret[0], rows[0]);
    }

    mat<nrows, ncols> invert() const { return invert_transpose().transpose(); }

    mat<ncols, nrows> transpose() const
    {
        mat<ncols, nrows> ret;
        for (size_t i = ncols; i--; ret[i] = this->col(i))
            ;
        return ret;
    }
};

template <size_t nrows, size_t ncols>
vec<nrows, double> operator*(const mat<nrows, ncols>& lhs, const vec<ncols, double>& rhs)
{
    vec<nrows, double> ret;
    for (size_t i = nrows; i--; ret[i] = dot(lhs[i], rhs))
        ;
    return ret;
}

template <size_t R1, size_t C1, size_t C2>
mat<R1, C2> operator*(const mat<R1, C1>& lhs, const mat<C1, C2>& rhs)
{
    mat<R1, C2> result;
    for (size_t i = R1; i--;)
        for (size_t j = C2; j--; result[i][j] = dot(lhs[i], rhs.col(j)))
            ;
    return result;
}

template <size_t nrows, size_t ncols>
mat<nrows, ncols> operator*(const mat<nrows, ncols>& lhs, const double& val)
{
    mat<nrows, ncols> result;
    for (size_t i = nrows; i--; result[i] = lhs[i] * val)
        ;
    return result;
}

template <size_t nrows, size_t ncols>
mat<nrows, ncols> operator/(const mat<nrows, ncols>& lhs, const double& val)
{
    mat<nrows, ncols> result;
    for (size_t i = nrows; i--; result[i] = lhs[i] / val)
        ;
    return result;
}

template <size_t nrows, size_t ncols>
mat<nrows, ncols> operator+(const mat<nrows, ncols>& lhs, const mat<nrows, ncols>& rhs)
{
    mat<nrows, ncols> result;
    for (size_t i = nrows; i--;)
        for (size_t j = ncols; j--; result[i][j] = lhs[i][j] + rhs[i][j])
            ;
    return result;
}

template <size_t nrows, size_t ncols>
mat<nrows, ncols> operator-(const mat<nrows, ncols>& lhs, const mat<nrows, ncols>& rhs)
{
    mat<nrows, ncols> result;
    for (size_t i = nrows; i--;)
        for (size_t j = ncols; j--; result[i][j] = lhs[i][j] - rhs[i][j])
            ;
    return result;
}

template <size_t nrows, size_t ncols>
std::ostream& operator<<(std::ostream& out, const mat<nrows, ncols>& m)
{
    for (size_t i = 0; i < nrows; i++) out << m[i] << std::endl;
    return out;
}

template <size_t n>
struct dt
{
    static double det(const mat<n, n>& src)
    {
        double ret = 0;
        for (size_t i = n; i--; ret += src[0][i] * src.cofactor(0, i))
            ;
        return ret;
    }
};

template <>
struct dt<1>
{
    static double det(const mat<1, 1>& src) { return src[0][0]; }
};

using vec2f = vec<2, double>;
using vec3f = vec<3, double>;
using vec4f = vec<4, double>;

using vec2i = vec<2, int>;
using vec3i = vec<3, int>;
using vec4i = vec<4, int>;

using mat4 = mat<4, 4>;

inline vec2f to_f(vec2i in) { return vec2f(static_cast<double>(in.x), static_cast<double>(in.y)); }
inline vec3f to_f(vec3i in)
{
    return vec3f(static_cast<double>(in.x), static_cast<double>(in.y), static_cast<double>(in.z));
}
inline vec4f to_f(vec4i in)
{
    return vec4f{static_cast<double>(in[0]), static_cast<double>(in[1]), static_cast<double>(in[2]),
                 static_cast<double>(in[3])};
}

inline vec3f cross(const vec3f& v1, const vec3f& v2)
{
    return vec3f{v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
}

template <typename T>
inline T clamp(T value, T minimum, T maximum)
{
    return std::min(std::max(value, minimum), maximum);
}