#pragma once
#ifndef _ENGINE_LIB_H
#define _ENGINE_LIB_H

#include <iostream>
#include <string>
#include <cmath>

#define DebugLog(x) std::cout << (x) << "\n";
void Log();
void Log(const std::string& out);
void Log(const char* out);
void Log(const char out);
void Log(const bool& out);
void Log(const int& out);
void Log(const unsigned int& out);
void Log(const float& out);
void Log(const double& out);

void FsReadDiskFile(std::string* content, const std::string& filePath);

template<typename T>
T templatedAbs(const T& scalar) {
	using std::abs;
	if constexpr(std::is_signed_v<T>) return static_cast<T>(abs(scalar));
	else return scalar;
}
template<typename T>
T templatedFloor(const T& scalar) {
	using std::floor;
	return static_cast<T>(floor(scalar));
}
template<typename T>
T templatedCeil(const T& scalar) {
	using std::ceil;
	return static_cast<T>(ceil(scalar));
}
template<typename T>
T templatedRound(const T& scalar) {
	using std::round;
	return static_cast<T>(round(scalar));
}
#pragma region vec2
template<typename T>
class vec2 {
	public:
	T x, y;
	vec2() : x(static_cast<T>(0)), y(static_cast<T>(0)) {};
	vec2(const T& _x, const T& _y) : x(_x), y(_y) {};
	explicit vec2(const T& scalar) : x(scalar), y(scalar) {};// from a single value
	vec2(const vec2<T>& copy) : x(copy.x), y(copy.y) {};
	vec2(vec2<T>&& move) noexcept : x(move.x), y(move.y) {};//nothing to actually move but implement the function anyways
	vec2<T> operator=(const vec2<T>& copy) { x=copy.x;y=copy.y; return copy; }
	vec2<T> operator=(vec2<T>&& move) noexcept { x=move.x;y=move.y; return move; }
	template<typename U>
	operator vec2<U>() const { return vec2<U>(static_cast<U>(x), static_cast<U>(y)); };
	void operator+=(const vec2<T>& rhs) { x+=rhs.x; y+=rhs.y; };
	void operator-=(const vec2<T>& rhs) { x-=rhs.x; y-=rhs.y; };
	T length() const { return static_cast<T>(sqrt(x*x+y*y)); };
	T magnitude() const { return static_cast<T>(sqrt(x*x+y*y)); };
	T sqrMagnitude() const { return x*x+y*y; };
	T cross(const vec2<T>& rhs) const { return x*rhs.y-y*rhs.x; };
	T dot(const vec2<T>& rhs) const { return (x*rhs.x)+(y*rhs.y); };
	vec2<T> normalized() const {
		T len=length();
		if(len==0) return vec2<T>::ZERO;
		else return vec2<T>(*this)/len;
	};
	vec2<T> abs() const { return vec2<T>(templatedAbs<T>(x), templatedAbs<T>(y)); };
	vec2<T> floor() const { return vec2<T>(templatedFloor<T>(x), templatedFloor<T>(y)); };
	vec2<T> ceil() const { return vec2<T>(templatedCeil<T>(x), templatedCeil<T>(y)); };
	vec2<T> round() const { return vec2<T>(templatedRound<T>(x), templatedRound<T>(y)); };
	vec2<T> operator+(const vec2<T>& rhs) const { return vec2<T>(x+rhs.x, y+rhs.y); };
	vec2<T> operator-(const vec2<T>& rhs) const { return vec2<T>(x-rhs.x, y-rhs.y); };
	template <typename U=T, std::enable_if_t<std::is_signed_v<U>, int>
	=0 >
	vec2<T> operator-() const { return vec2<T>(-x, -y); };
	template<typename U>
	vec2<T> operator*(const U& scalar) const { return vec2<T>(x*static_cast<T>(scalar), y*static_cast<T>(scalar)); };
	template<typename U>
	friend vec2<T> operator*(const U& scalar, const vec2<T>& vector) { return vector*static_cast<T>(scalar); }
	template<typename U>
	vec2<T> operator/(const U& scalar) const { return vec2<T>(x/static_cast<T>(scalar), y/static_cast<T>(scalar)); };
	T operator[](const int& i) const {
		if(i==0) return x;
		else if(i==1) return y;
		else return static_cast<T>(0);
	};
	bool operator==(const vec2<T>& rhs) const { return (x==rhs.x)&&(y==rhs.y); };
	bool operator!=(const vec2<T>& rhs) const { return (x!=rhs.x)||(y!=rhs.y); };
	operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+")"; };
	friend std::ostream& operator<<(std::ostream& o, const vec2<T>& rhs) {
		o<<"("<<rhs.x<<", "<<rhs.y<<")"; return o;
	}
	friend void Log(const vec2<T>& a) {
#ifdef _ENGINE_DEBUG
		DebugLog(a)
#endif // _ENGINE_DEBUG
	}

	static const vec2<T> ZERO;
	static const vec2<T> ONE;
	static const vec2<T> RIGHT;
	static const vec2<T> UP;

	static const vec2<T> Center;
	static const vec2<T> TopRight;
	static const vec2<T> TopCenter;
	static const vec2<T> TopLeft;
	static const vec2<T> LeftCenter;
	static const vec2<T> BottomLeft;
	static const vec2<T> BottomCenter;
	static const vec2<T> BottomRight;
	static const vec2<T> RightCenter;
};
template<typename T>
const vec2<T> vec2<T>::ZERO=vec2<T>(static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec2<T> vec2<T>::ONE=vec2<T>(static_cast<T>(1), static_cast<T>(1));
template<typename T>
const vec2<T> vec2<T>::RIGHT=vec2<T>(static_cast<T>(1), static_cast<T>(0));
template<typename T>
const vec2<T> vec2<T>::UP=vec2<T>(static_cast<T>(0), static_cast<T>(1));
template<typename T>
const vec2<T> vec2<T>::Center=vec2<T>(static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec2<T> vec2<T>::TopRight=vec2<T>(static_cast<T>(0.5f), static_cast<T>(0.5f));
template<typename T>
const vec2<T> vec2<T>::TopCenter=vec2<T>(static_cast<T>(0), static_cast<T>(0.5f));
template<typename T>
const vec2<T> vec2<T>::TopLeft=vec2<T>(static_cast<T>(-0.5f), static_cast<T>(0.5f));
template<typename T>
const vec2<T> vec2<T>::LeftCenter=vec2<T>(static_cast<T>(-0.5f), static_cast<T>(0));
template<typename T>
const vec2<T> vec2<T>::BottomLeft=vec2<T>(static_cast<T>(-0.5f), static_cast<T>(-0.5f));
template<typename T>
const vec2<T> vec2<T>::BottomCenter=vec2<T>(static_cast<T>(0), static_cast<T>(-0.5f));
template<typename T>
const vec2<T> vec2<T>::BottomRight=vec2<T>(static_cast<T>(0.5f), static_cast<T>(-0.5f));
template<typename T>
const vec2<T> vec2<T>::RightCenter=vec2<T>(static_cast<T>(0.5f), static_cast<T>(0));
typedef vec2<float> Vector2;
typedef vec2<float> Vector2f;
typedef vec2<double> Vector2d;
typedef vec2<int> Vector2i;
typedef vec2<unsigned int> Vector2ui;
#pragma endregion// vec2

#pragma region vec3
template<typename T>
class vec3 {
	public:
	T x, y, z;
	vec3() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)) {};
	vec3(const T& _x, const T& _y, const T& _z) : x(_x), y(_y), z(_z) {};
	explicit vec3(const T& scalar) : x(scalar), y(scalar), z(scalar) {};// from a single value
	vec3(const vec2<T>& xy, const T& _z) : x(xy.x), y(xy.y), z(_z) {};
	vec3(const T& _x, const vec2<T>& yz) : x(_x), y(yz.x), z(yz.y) {};
	vec3(const vec3<T>& copy) : x(copy.x), y(copy.y), z(copy.z) {};
	vec3(vec3<T>&& move) noexcept : x(move.x), y(move.y), z(move.z) {};// nothing to actually move but allow the function to be used
	vec3<T> operator=(const vec3<T>& copy) { x=copy.x;y=copy.y;z=copy.z; return copy; };
	vec3<T> operator=(vec3<T>&& move) noexcept { x=move.x;y=move.y;z=move.z; return move; };
	vec2<T> XY() { return vec2<T>(x, y); };
	vec2<T> YZ() { return vec2<T>(y, z); };
	template<typename U>
	operator vec3<U>() const { return vec3<U>(static_cast<U>(x), static_cast<U>(y)); };
	void operator+=(const vec3<T>& rhs) { x+=rhs.x; y+=rhs.y; };
	void operator-=(const vec3<T>& rhs) { x-=rhs.x; y-=rhs.y; };
	T length() const { return static_cast<T>(sqrt(x*x+y*y+z*z)); };
	T magnitude() const { return static_cast<T>(sqrt(x*x+y*y+z*z)); };
	T sqrMagnitude() const { return x*x+y*y+z*z; };
	vec3<T> cross(const vec3<T>& rhs) const { return vec3<T>((y*rhs.z-z*rhs.y), (z*rhs.x-x*rhs.z), (x*rhs.y-y*rhs.x)); };
	T dot(const vec3<T>& rhs) const { return (x*rhs.x)+(y*rhs.y)+(z*rhs.z); };
	vec3<T> normalized() const {
		T len=length();
		if(len==0) return vec3<T>::ZERO;
		else return vec3<T>(*this)/len;
	};
	vec3<T> abs() const { return vec3<T>(templatedAbs<T>(x), templatedAbs<T>(y), templatedAbs<T>(z)); };
	vec3<T> floor() const { return vec3<T>(templatedFloor<T>(x), templatedFloor<T>(y), templatedFloor<T>(z)); };
	vec3<T> ceil() const { return vec3<T>(templatedCeil<T>(x), templatedCeil<T>(y), templatedCeil<T>(z)); };
	vec3<T> round() const { return vec3<T>(templatedRound<T>(x), templatedRound<T>(y), templatedRound<T>(z)); };
	vec3<T> operator+(const vec3<T>& rhs) const { return vec3<T>(x+rhs.x, y+rhs.y, z+rhs.z); };
	vec3<T> operator-(const vec3<T>& rhs) const { return vec3<T>(x-rhs.x, y-rhs.y, z-rhs.z); };
	template <typename U=T, std::enable_if_t<std::is_signed_v<U>, int>
	=0 >
	vec3<T> operator-() const { return vec3<T>(-x, -y, -z); };
	template<typename U>
	vec3<T> operator*(const U& scalar) const { return vec3<T>(x*static_cast<T>(scalar), y*static_cast<T>(scalar), z*static_cast<T>(scalar)); };
	template<typename U>
	friend vec3<T> operator*(const U& scalar, const vec3<T>& vector) { return vector*static_cast<T>(scalar); };
	template<typename U>
	vec3<T> operator/(const U& scalar) const { return vec3<T>(x/static_cast<T>(scalar), y/static_cast<T>(scalar), z/static_cast<T>(scalar)); };
	T operator[](const int& i) const {
		if(i==0) return x;
		else if(i==1) return y;
		else if(i==2) return z;
		else return static_cast<T>(0);
	};
	bool operator==(const vec3<T>& rhs) const { return (x==rhs.x)&&(y==rhs.y)&&(z==rhs.z); };
	bool operator!=(const vec3<T>& rhs) const { return (x!=rhs.x)||(y!=rhs.y)||(z!=rhs.z); };
	operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")"; };
	friend std::ostream& operator<<(std::ostream& o, const vec3<T>& rhs) {
		o<<"("<<rhs.x<<", "<<rhs.y<<", "<<rhs.z<<")"; return o;
	};
	friend void Log(const vec3<T>& a) {
#ifdef _ENGINE_DEBUG
		DebugLog(a)
#endif // _ENGINE_DEBUG
	};
	static const vec3<T> ZERO;
	static const vec3<T> ONE;
	static const vec3<T> RIGHT;
	static const vec3<T> UP;
	static const vec3<T> FORWARD;
};
template<typename T>
const vec3<T> vec3<T>::ZERO=vec3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec3<T> vec3<T>::ONE=vec3<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));
template<typename T>
const vec3<T> vec3<T>::RIGHT=vec3<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec3<T> vec3<T>::UP=vec3<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
template<typename T>
const vec3<T> vec3<T>::FORWARD=vec3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
typedef vec3<float> Vector3;
typedef vec3<float> Vector3f;
typedef vec3<double> Vector3d;
typedef vec3<int> Vector3i;
typedef vec3<unsigned int> Vector3ui;
#pragma endregion// vec3

#pragma region vec4
template<typename T>
class vec4 {
	public:
	T x, y, z, w;
	vec4() : x(static_cast<T>(0)), y(static_cast<T>(0)), z(static_cast<T>(0)), w(static_cast<T>(0)) {};
	vec4(const T& _x, const T& _y, const T& _z, const T& _w) : x(_x), y(_y), z(_z), w(_w) {};
	explicit vec4(const T& scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {};// from a single value

	vec4(const vec2<T>& xy, const T& _z, const T& _w) : x(xy.x), y(xy.y), z(_z), w(_w) {};
	vec4(const T& _x, const vec2<T>& yz, const T& _w) : x(_x), y(yz.x), z(yz.y), w(_w) {};
	vec4(const T& _x, const T& _y, const vec2<T>& zw) : x(_x), y(_y), z(zw.x), w(zw.y) {};
	vec4(const vec2<T>& xy, const vec2<T>& zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {};
	vec4(const vec3<T>& xyz, const T& _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {};
	vec4(const T& _x, const vec3<T>& yzw) : x(_x), y(yzw.x), z(yzw.y), w(yzw.z) {};

	vec4(const vec4<T>& copy) : x(copy.x), y(copy.y), z(copy.z), w(copy.w) {};
	vec4(vec4<T>&& move) noexcept : x(move.x), y(move.y), z(move.z), w(move.w) {};// nothing to actually move but allow the function to be used
	vec4<T> operator=(const vec4<T>& copy) { x=copy.x;y=copy.y;z=copy.z;w=copy.w; return copy; };
	vec4<T> operator=(vec4<T>&& move) noexcept { x=move.x;y=move.y;z=move.z;w=move.w; return move; };
	vec3<T> XYZ() { return vec3<T>(x, y, z); };
	vec3<T> YZW() { return vec3<T>(y, z, w); };
	vec2<T> XY() { return vec2<T>(x, y); };
	vec2<T> YZ() { return vec2<T>(y, z); };
	vec2<T> ZW() { return vec2<T>(z, w); };
	template<typename U>
	operator vec4<U>() const { return vec4<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(w)); };
	void operator+=(const vec4<T>& rhs) { x+=rhs.x; y+=rhs.y; w+=rhs.w; };
	void operator-=(const vec4<T>& rhs) { x-=rhs.x; y-=rhs.y; w-=rhs.w; };
	T length() const { return static_cast<T>(sqrt(x*x+y*y+z*z+w*w)); };
	T magnitude() const { return static_cast<T>(sqrt(x*x+y*y+z*z+w*w)); };
	T sqrMagnitude() const { return x*x+y*y+z*z+w*w; };
	T dot(const vec4<T>& rhs) const { return (x*rhs.x)+(y*rhs.y)+(z*rhs.z)+(w*rhs.w); };
	vec4<T> normalized() const {
		T len=length();
		if(len==0) return vec4<T>::ZERO;
		else return vec4<T>(*this)/len;
	};
	vec4<T> abs() const { return vec4<T>(templatedAbs<T>(x), templatedAbs<T>(y), templatedAbs<T>(z), templatedAbs<T>(w)); };
	vec4<T> floor() const { return vec4<T>(templatedFloor<T>(x), templatedFloor<T>(y), templatedFloor<T>(z), templatedFloor<T>(w)); };
	vec4<T> ceil() const { return vec4<T>(templatedCeil<T>(x), templatedCeil<T>(y), templatedCeil<T>(z), templatedCeil<T>(w)); };
	vec4<T> round() const { return vec4<T>(templatedRound<T>(x), templatedRound<T>(y), templatedRound<T>(z), templatedRound<T>(w)); };
	vec4<T> operator+(const vec4<T>& rhs) const { return vec4<T>(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w); };
	vec4<T> operator-(const vec4<T>& rhs) const { return vec4<T>(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w); };
	template <typename U=T, std::enable_if_t<std::is_signed_v<U>, int>
	=0 >
	vec4<T> operator-() const { return vec4<T>(-x, -y, -z, -w); };
	template<typename U>
	vec4<T> operator*(const U& scalar) const { return vec4<T>(x*static_cast<T>(scalar), y*static_cast<T>(scalar), z*static_cast<T>(scalar), w*static_cast<T>(scalar)); };
	template<typename U>
	friend vec4<T> operator*(const U& scalar, const vec4<T>& vector) { return vector*static_cast<T>(scalar); };
	template<typename U>
	vec4<T> operator/(const U& scalar) const { return vec4<T>(x/static_cast<T>(scalar), y/static_cast<T>(scalar), z/static_cast<T>(scalar), w/static_cast<T>(scalar)); };
	T operator[](const int& i) const {
		if(i==0) return x;
		else if(i==1) return y;
		else if(i==2) return z;
		else if(i==3) return w;
		else return static_cast<T>(0);
	};
	bool operator==(const vec4<T>& rhs) const { return (x==rhs.x)&&(y==rhs.y)&&(z==rhs.z)&&(w==rhs.w); };
	bool operator!=(const vec4<T>& rhs) const { return (x!=rhs.x)||(y!=rhs.y)||(z!=rhs.z)||(w!=rhs.w); };
	operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")"; };
	friend std::ostream& operator<<(std::ostream& o, const vec4<T>& rhs) {
		o<<"("<<rhs.x<<", "<<rhs.y<<", "<<rhs.z<<", "<<rhs.w<<")"; return o;
	};
	friend void Log(const vec4<T>& a) {
#ifdef _ENGINE_DEBUG
		DebugLog(a)
#endif // _ENGINE_DEBUG
	};
	static const vec4<T> ZERO;
	static const vec4<T> ONE;
	static const vec4<T> RIGHT;
	static const vec4<T> UP;
	static const vec4<T> FORWARD;
	static const vec4<T> ANA;
};
template<typename T>
const vec4<T> vec4<T>::ZERO=vec4<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec4<T> vec4<T>::ONE=vec4<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));
template<typename T>
const vec4<T> vec4<T>::RIGHT=vec4<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec4<T> vec4<T>::UP=vec4<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));
template<typename T>
const vec4<T> vec4<T>::FORWARD=vec4<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
template<typename T>
const vec4<T> vec4<T>::ANA=vec4<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
typedef vec4<float> Vector4;
typedef vec4<float> Vector4f;
typedef vec4<double> Vector4d;
typedef vec4<int> Vector4i;
typedef vec4<unsigned int> Vector4ui;
#pragma endregion// vec4

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
	operator std::string() const;
	friend std::ostream& operator<<(std::ostream& o, const Mat4x4& rhs) {
		o<<"[ "+std::to_string(rhs.get(0, 0))+", "+std::to_string(rhs.get(1, 0))+", "+std::to_string(rhs.get(2, 0))+", "+std::to_string(rhs.get(3, 0))+","+
			"\n  "+std::to_string(rhs.get(0, 1))+", "+std::to_string(rhs.get(1, 1))+", "+std::to_string(rhs.get(2, 1))+", "+std::to_string(rhs.get(3, 1))+","+
			"\n  "+std::to_string(rhs.get(0, 2))+", "+std::to_string(rhs.get(1, 2))+", "+std::to_string(rhs.get(2, 2))+", "+std::to_string(rhs.get(3, 2))+","+
			"\n  "+std::to_string(rhs.get(0, 3))+", "+std::to_string(rhs.get(1, 3))+", "+std::to_string(rhs.get(2, 3))+", "+std::to_string(rhs.get(3, 3))+"]";
		return o;
	}
	friend void Log(const Mat4x4& a) {
#ifdef _ENGINE_DEBUG
		DebugLog(a)
#endif // _ENGINE_DEBUG
	};
};
float deg_to_rad(const float& deg);
float rad_to_deg(const float& deg);
Mat4x4 translate(const Vector3& value);
Mat4x4 scaleMat(const Vector3& value);
Mat4x4 axisRotMat(const Vector3& axis, const float& angle);
Mat4x4 ortho(const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far);
Mat4x4 perspective(const float& fov, const float& aspect, const float& near, const float& far);
Mat4x4 lookAt(const Vector3& position, const Vector3& target, const Vector3& up);

namespace std {
	template<typename T>
	string to_string(const vec2<T>& a) { return (std::string)a; };
	template<typename T>
	string to_string(const vec3<T>& a) { return (std::string)a; };
	template<typename T>
	string to_string(const vec4<T>& a) { return (std::string)a; };
}
#endif// _ENGINE_LIB_H