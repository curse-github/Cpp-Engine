#pragma once
#ifndef _LIBH
#define _LIBH

#include <iostream>
#include <string>
using namespace std;

void Log(string out);
void Log(const char* out);
void Log(bool out);
void Log(int out);
void Log(float out);
void Log(double out);

class Vector2 {
	public:
	float x;
	float y;
	Vector2();
	Vector2(float _x, float _y);
	Vector2(float v);
	void operator+=(Vector2 b);
	void operator-=(Vector2 b);
	float length();
	float dot(Vector2 b);
	Vector2 normalized();
	Vector2 operator+(Vector2 b);
	Vector2 operator-(Vector2 b);
	Vector2 operator-();
	Vector2 operator*(float b);
	Vector2 operator*(int b);
	Vector2 operator/(float b);
	Vector2 operator/(int b);
};

class Vector3 {
	public:
	float x;
	float y;
	float z;
	Vector3();
	Vector3(Vector2 v, float _z);
	Vector3(float _x, float _y, float _z);
	Vector3(float v);
	Vector3 cross(Vector3 b);
	void operator+=(Vector3 b);
	void operator-=(Vector3 b);
	float length();
	float dot(Vector3 b);
	Vector3 normalized();
	Vector3 operator+(Vector3 b);
	Vector3 operator-(Vector3 b);
	Vector3 operator-();
	Vector3 operator*(float b);
	Vector3 operator*(int b);
	Vector3 operator/(float b);
	Vector3 operator/(int b);
};
class Vector4 {
	public:
	float x;
	float y;
	float z;
	float w;
	Vector4();
	Vector4(Vector3 v, float _w);
	Vector4(float _x, float _y, float _z, float _w);
	Vector4(float v);
	Vector3 toXYZ();
	void operator+=(Vector4 b);
	void operator-=(Vector4 b);
	float length();
	float dot(Vector4 b);
	Vector4 normalized();
	Vector4 operator+(Vector4 b);
	Vector4 operator-(Vector4 b);
	Vector4 operator-();
	Vector4 operator*(float b);
	Vector4 operator*(int b);
	Vector4 operator/(float b);
	Vector4 operator/(int b);
};
class Mat4x4 {
	public:
	float values[16];
	Mat4x4(float _values[16]);
	Mat4x4();
	float get(int x, int y);
	float operator[](int i);
	Vector4 getColumn(int x);
	Vector4 getRow(int y);
	Vector4 operator*(Vector4 b);
	Mat4x4 operator*(Mat4x4 b);
};
float deg_to_rad(float deg);
float rad_to_deg(float deg);
Mat4x4 translate(Vector3 value);
Mat4x4 axisRotMat(Vector3 axis, float angle);
Mat4x4 ortho(float left, float right, float bottom, float top, float near, float far);
Mat4x4 perspective(float fov, float aspect, float near, float far);
Mat4x4 lookAt(Vector3 position, Vector3 target, Vector3 up);

#endif