#pragma once

#include <cmath>

struct Vec3
{
    float x;
    float y;
    float z;
};

struct Mat4
{
    float m[4][4];
};

inline float Vec3Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 Vec3Subtract(const Vec3& a, const Vec3& b)
{
    Vec3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

inline Vec3 Vec3Cross(const Vec3& a, const Vec3& b)
{
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

inline Vec3 Vec3Normalize(const Vec3& value)
{
    float length = std::sqrt(Vec3Dot(value, value));
    if (length <= 0.000001f)
    {
        Vec3 zero = { 0.0f, 0.0f, 0.0f };
        return zero;
    }
    Vec3 result = { value.x / length, value.y / length, value.z / length };
    return result;
}

inline Mat4 MatrixIdentity()
{
    Mat4 result = {};
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

inline Mat4 MatrixMultiply(const Mat4& a, const Mat4& b)
{
    Mat4 result = {};
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            for (int k = 0; k < 4; ++k)
            {
                result.m[row][column] += a.m[row][k] * b.m[k][column];
            }
        }
    }
    return result;
}

inline Mat4 MatrixTranslation(float x, float y, float z)
{
    Mat4 result = MatrixIdentity();
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

inline Mat4 MatrixScale(float x, float y, float z)
{
    Mat4 result = {};
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    result.m[3][3] = 1.0f;
    return result;
}

inline Mat4 MatrixRotationX(float angle)
{
    Mat4 result = MatrixIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[1][1] = c;
    result.m[1][2] = s;
    result.m[2][1] = -s;
    result.m[2][2] = c;
    return result;
}

inline Mat4 MatrixRotationY(float angle)
{
    Mat4 result = MatrixIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][2] = -s;
    result.m[2][0] = s;
    result.m[2][2] = c;
    return result;
}

inline Mat4 MatrixRotationZ(float angle)
{
    Mat4 result = MatrixIdentity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] = c;
    result.m[0][1] = s;
    result.m[1][0] = -s;
    result.m[1][1] = c;
    return result;
}

inline Mat4 MatrixRotationXYZ(float x, float y, float z)
{
    return MatrixMultiply(MatrixMultiply(MatrixRotationX(x), MatrixRotationY(y)), MatrixRotationZ(z));
}

inline Mat4 MatrixLookAtLH(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    Vec3 zAxis = Vec3Normalize(Vec3Subtract(target, eye));
    Vec3 xAxis = Vec3Normalize(Vec3Cross(up, zAxis));
    Vec3 yAxis = Vec3Cross(zAxis, xAxis);
    Mat4 result = {};
    result.m[0][0] = xAxis.x; result.m[0][1] = yAxis.x; result.m[0][2] = zAxis.x;
    result.m[1][0] = xAxis.y; result.m[1][1] = yAxis.y; result.m[1][2] = zAxis.y;
    result.m[2][0] = xAxis.z; result.m[2][1] = yAxis.z; result.m[2][2] = zAxis.z;
    result.m[3][0] = -Vec3Dot(xAxis, eye);
    result.m[3][1] = -Vec3Dot(yAxis, eye);
    result.m[3][2] = -Vec3Dot(zAxis, eye);
    result.m[3][3] = 1.0f;
    return result;
}

inline Mat4 MatrixPerspectiveFovLH(float fovY, float aspect, float nearPlane, float farPlane)
{
    Mat4 result = {};
    float yScale = 1.0f / std::tan(fovY * 0.5f);
    float xScale = yScale / aspect;
    result.m[0][0] = xScale;
    result.m[1][1] = yScale;
    result.m[2][2] = farPlane / (farPlane - nearPlane);
    result.m[2][3] = 1.0f;
    result.m[3][2] = (-nearPlane * farPlane) / (farPlane - nearPlane);
    return result;
}
