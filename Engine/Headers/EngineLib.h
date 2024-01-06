#pragma once
#ifndef _LIB_H
#define _LIB_H

#include <iostream>
#include <string>
#include <fstream>

#define DebugLog(x) std::cout << (x) << "\n";
void Log(const std::string& out);
void Log(const char* out);
void Log(const char out);
void Log(const bool& out);
void Log(const int& out);
void Log(const unsigned int& out);
void Log(const float& out);
void Log(const double& out);
class Vector2;
class Vector3;
class Vector4;
class Mat4x4;
void Log(const Vector2& out);
void Log(const Vector3& out);
void Log(const Vector4& out);
void Log(const Mat4x4& out);

void FsReadDiskFile(std::string* content, const std::string& filePath);

class Vector2 {
	public:
	float x;
	float y;
	Vector2();
	Vector2(const float& _x, const float& _y);
	Vector2(const float& v);
	void operator+=(const Vector2& b);
	void operator-=(const Vector2& b);
	float length() const;
	float magnitude() const;
	float sqrMagnitude() const;
	float cross(const Vector2& b) const;
	float dot(const Vector2& b) const;
	Vector2 normalized() const;
	Vector2 abs() const;
	Vector2 floor() const;
	Vector2 round() const;
	Vector2 ceil() const;
	Vector2 operator+(const Vector2& b) const;
	Vector2 operator-(const Vector2& b) const;
	Vector2 operator-() const;
	Vector2 operator*(const float& b) const;
	Vector2 operator*(const double& b) const;
	Vector2 operator*(const int& b) const;
	Vector2 operator/(const float& b) const;
	Vector2 operator/(const double& b) const;
	Vector2 operator/(const int& b) const;
	float operator[](const int& i) const;
	bool operator==(const Vector2& b) const;
	bool operator!=(const Vector2& b) const;
	std::string to_string() const;
	operator std::string() const;
	friend std::ostream& operator<<(std::ostream& o, const Vector2& rhs) {
		o<<rhs.to_string(); return o;
	}
	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 RIGHT;
	static const Vector2 UP;

	static const Vector2 Center;
	static const Vector2 TopRight;
	static const Vector2 TopCenter;
	static const Vector2 TopLeft;
	static const Vector2 LeftCenter;
	static const Vector2 BottomLeft;
	static const Vector2 BottomCenter;
	static const Vector2 BottomRight;
	static const Vector2 RightCenter;
};

class Vector3 {
	public:
	float x;
	float y;
	float z;
	Vector3();
	Vector3(const Vector2& v, const float& _z);
	Vector3(const float& _x, const float& _y, const float& _z);
	Vector3(const float& v);
	Vector2 toXY() const;
	void operator+=(const Vector3& b);
	void operator-=(const Vector3& b);
	float length() const;
	Vector3 cross(const Vector3& b) const;
	float dot(const Vector3& b) const;
	Vector3 normalized() const;
	Vector3 abs() const;
	Vector3 floor() const;
	Vector3 round() const;
	Vector3 ceil() const;
	Vector3 operator+(const Vector3& b);
	Vector3 operator-(const Vector3& b) const;
	Vector3 operator-() const;
	Vector3 operator*(const float& b) const;
	Vector3 operator*(const double& b) const;
	Vector3 operator*(const int& b) const;
	Vector3 operator/(const float& b) const;
	Vector3 operator/(const double& b) const;
	Vector3 operator/(const int& b) const;
	float operator[](const int& i) const;
	bool operator==(const Vector3& b) const;
	bool operator!=(const Vector3& b) const;
	std::string to_string() const;
	operator std::string() const;
	friend std::ostream& operator<<(std::ostream& o, const Vector3& rhs) {
		o<<rhs.to_string(); return o;
	}
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
	Vector4(const Vector3& v, const float& _w);
	Vector4(const float& _x, const float& _y, const float& _z, const float& _w);
	Vector4(const float& v);
	Vector3 toXYZ() const;
	void operator+=(const Vector4& b);
	void operator-=(const Vector4& b);
	float length() const;
	float dot(const Vector4& b) const;
	Vector4 normalized() const;
	Vector4 abs() const;
	Vector4 floor() const;
	Vector4 round() const;
	Vector4 ceil() const;
	Vector4 operator+(const Vector4& b) const;
	Vector4 operator-(const Vector4& b) const;
	Vector4 operator-() const;
	Vector4 operator*(const float& b) const;
	Vector4 operator*(const double& b) const;
	Vector4 operator*(const int& b) const;
	Vector4 operator/(const float& b) const;
	Vector4 operator/(const double& b) const;
	Vector4 operator/(const int& b) const;
	float operator[](const int& i) const;
	bool operator==(const Vector4& b) const;
	bool operator!=(const Vector4& b) const;
	std::string to_string() const;
	operator std::string() const;
	friend std::ostream& operator<<(std::ostream& o, const Vector4& rhs) {
		o<<rhs.to_string(); return o;
	}
	static const Vector4 ZERO;
	static const Vector4 ONE;
	static const Vector4 RIGHT;
	static const Vector4 UP;
	static const Vector4 FORWARD;
	static const Vector4 ANA;
};

class Mat4x4 {
	public:
	bool isIdentity=false;
	float values[16];
	Mat4x4(float _values[16]);
	Mat4x4();
	float get(const int& x, const int& y) const;
	float operator[](const int& i) const;
	Vector4 getColumn(const int& x) const;
	Vector4 getRow(const int& y) const;
	Vector4 operator*(const Vector4& b) const;
	Vector3 operator*(const Vector3& b) const;
	Mat4x4 operator*(const Mat4x4& b) const;
	std::string to_string() const;
	friend std::ostream& operator<<(std::ostream& o, const Mat4x4& rhs) {
		o<<rhs.to_string(); return o;
	}
};
float deg_to_rad(const float& deg);
float rad_to_deg(const float& deg);
Mat4x4 translate(const Vector3& value);
Mat4x4 scaleMat(const Vector3& value);
Mat4x4 axisRotMat(const Vector3& axis, const float& angle);
Mat4x4 ortho(const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far);
Mat4x4 perspective(const float& fov, const float& aspect, const float& near, const float& far);
Mat4x4 lookAt(const Vector3& position, const Vector3& target, const Vector3& up);

#endif// _LIB_H