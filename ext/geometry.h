#pragma once

#include <cmath>
#include <cassert>
#include <iostream>
#include <fstream>

template <size_t n>
struct vec
{
    vec() = default;
    double& operator[](const size_t i)
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
    double operator[](const size_t i) const
    {
        assert(i >= 0 && i < n);
        return data[i];
    }
    double norm2() const { return (*this) * (*this); }
    double norm() const { return std::sqrt(norm2()); }
    double data[n] = {0};
};

template <size_t n>
double operator*(const vec<n>& lhs, const vec<n>& rhs)
{
    double ret = 0;
    for (size_t i = n; i--; ret += lhs[i] * rhs[i])
        ;
    return ret;
}

template <size_t n>
vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] += rhs[i])
        ;
    return ret;
}

template <size_t n>
vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] -= rhs[i])
        ;
    return ret;
}

template <size_t n>
vec<n> operator*(const double& rhs, const vec<n>& lhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] *= rhs)
        ;
    return ret;
}

template <size_t n>
vec<n> operator*(const vec<n>& lhs, const double& rhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] *= rhs)
        ;
    return ret;
}

template <size_t n>
vec<n> operator/(const vec<n>& lhs, const double& rhs)
{
    vec<n> ret = lhs;
    for (size_t i = n; i--; ret[i] /= rhs)
        ;
    return ret;
}

template <size_t n1, size_t n2>
vec<n1> embed(const vec<n2>& v, double fill = 1)
{
    vec<n1> ret;
    for (size_t i = n1; i--; ret[i] = (i < n2 ? v[i] : fill))
        ;
    return ret;
}

template <size_t n1, size_t n2>
vec<n1> proj(const vec<n2>& v)
{
    vec<n1> ret;
    for (size_t i = n1; i--; ret[i] = v[i])
        ;
    return ret;
}

template <size_t n>
std::ostream& operator<<(std::ostream& out, const vec<n>& v)
{
    for (size_t i = 0; i < n; i++) out << v[i] << " ";
    return out;
}

template <>
struct vec<2>
{
    vec() = default;
    vec(double X, double Y) : x(X), y(Y) {}
    double& operator[](const size_t i)
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }
    double operator[](const size_t i) const
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }
    double norm2() const { return (*this) * (*this); }
    double norm() const { return std::sqrt(norm2()); }
    vec& normalize()
    {
        *this = (*this) / norm();
        return *this;
    }

    double x{}, y{};
};

template <>
struct vec<3>
{
    vec() = default;
    vec(double X, double Y, double Z) : x(X), y(Y), z(Z) {}
    double& operator[](const size_t i)
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (1 == i ? y : z);
    }
    double operator[](const size_t i) const
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (1 == i ? y : z);
    }
    double norm2() const { return (*this) * (*this); }
    double norm() const { return std::sqrt(norm2()); }
    vec& normalize()
    {
        *this = (*this) / norm();
        return *this;
    }

    double x{}, y{}, z{};
};

template <size_t n>
struct dt;

template <size_t nrows, size_t ncols>
struct mat
{
    vec<ncols> rows[nrows] = {{}};

    mat() = default;
    vec<ncols>& operator[](const size_t idx)
    {
        assert(idx >= 0 && idx < nrows);
        return rows[idx];
    }
    const vec<ncols>& operator[](const size_t idx) const
    {
        assert(idx >= 0 && idx < nrows);
        return rows[idx];
    }

    vec<nrows> col(const size_t idx) const
    {
        assert(idx >= 0 && idx < ncols);
        vec<nrows> ret;
        for (size_t i = nrows; i--; ret[i] = rows[i][idx])
            ;
        return ret;
    }

    void set_col(const size_t idx, const vec<nrows>& v)
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
        return ret / (ret[0] * rows[0]);
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
vec<nrows> operator*(const mat<nrows, ncols>& lhs, const vec<ncols>& rhs)
{
    vec<nrows> ret;
    for (size_t i = nrows; i--; ret[i] = lhs[i] * rhs)
        ;
    return ret;
}

template <size_t R1, size_t C1, size_t C2>
mat<R1, C2> operator*(const mat<R1, C1>& lhs, const mat<C1, C2>& rhs)
{
    mat<R1, C2> result;
    for (size_t i = R1; i--;)
        for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j))
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

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

inline vec3 cross(const vec3& v1, const vec3& v2)
{
    return vec<3>{v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x};
}
