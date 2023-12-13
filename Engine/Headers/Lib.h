#pragma once
#ifndef _LIBH
#define _LIBH

#include <iostream>
#include <string>
#include <fstream>

void Log(std::string out);
void Log(const char* out);
void Log(bool out);
void Log(int out);
void Log(unsigned int out);
void Log(float out);
void Log(double out);
class Vector2; class Vector3; class Vector4;
void Log(Vector2 out);
void Log(Vector3 out);
void Log(Vector4 out);
class Mat4x4;
void Log(Mat4x4 out);

void FsReadDiskFile(std::string* content, const std::string& filePath);

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
	Vector2 abs();
	Vector2 floor();
	Vector2 round();
	Vector2 ceil();
	Vector2 operator+(Vector2 b);
	Vector2 operator-(Vector2 b);
	Vector2 operator-();
	Vector2 operator*(float b);
	Vector2 operator*(double b);
	Vector2 operator*(int b);
	Vector2 operator/(float b);
	Vector2 operator/(double b);
	Vector2 operator/(int b);
	float operator[](int i);
	bool operator==(Vector2 b);
	bool operator!=(Vector2 b);
	std::string to_string();
	operator std::string() const;
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 RIGHT;
	static const Vector2 UP;
	static const Vector2 Center;
	static const Vector2 TopRight;
	static const Vector2 TopLeft;
	static const Vector2 BottomLeft;
	static const Vector2 BottomRight;
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
	Vector2 toXY();
	Vector3 cross(Vector3 b);
	void operator+=(Vector3 b);
	void operator-=(Vector3 b);
	float length();
	float dot(Vector3 b);
	Vector3 normalized();
	Vector3 abs();
	Vector3 floor();
	Vector3 round();
	Vector3 ceil();
	Vector3 operator+(Vector3 b);
	Vector3 operator-(Vector3 b);
	Vector3 operator-();
	Vector3 operator*(float b);
	Vector3 operator*(double b);
	Vector3 operator*(int b);
	Vector3 operator/(float b);
	Vector3 operator/(double b);
	Vector3 operator/(int b);
	float operator[](int i);
	bool operator==(Vector3 b);
	bool operator!=(Vector3 b);
	std::string to_string();
	operator std::string() const;
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 RIGHT;
	static const Vector3 UP;
	static const Vector3 FORWARD;
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
	Vector4 abs();
	Vector4 floor();
	Vector4 round();
	Vector4 ceil();
	Vector4 operator+(Vector4 b);
	Vector4 operator-(Vector4 b);
	Vector4 operator-();
	Vector4 operator*(float b);
	Vector4 operator*(double b);
	Vector4 operator*(int b);
	Vector4 operator/(float b);
	Vector4 operator/(double b);
	Vector4 operator/(int b);
	float operator[](int i);
	bool operator==(Vector4 b);
	bool operator!=(Vector4 b);
	std::string to_string();
	operator std::string() const;
	static const Vector4 ZERO;
	static const Vector4 ONE;
	static const Vector4 RIGHT;
	static const Vector4 UP;
	static const Vector4 FORWARD;
	static const Vector4 ANA;
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
	std::string to_string();
};
float deg_to_rad(float deg);
float rad_to_deg(float deg);
Mat4x4 translate(Vector3 value);
Mat4x4 scaleMat(Vector3 value);
Mat4x4 axisRotMat(Vector3 axis, float angle);
Mat4x4 ortho(float left, float right, float bottom, float top, float near, float far);
Mat4x4 perspective(float fov, float aspect, float near, float far);
Mat4x4 lookAt(Vector3 position, Vector3 target, Vector3 up);
#endif