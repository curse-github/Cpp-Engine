#include "EngineLib.h"
#include <fstream>

void Log() {
#ifdef _DEBUG
	DebugLog("")
#endif // _DEBUG
}
void Log(const std::string& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const char* out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const char out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const bool& out) {
#ifdef _DEBUG
	DebugLog(out ? "True" : "False")
#endif // _DEBUG
}
void Log(const int& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const unsigned int& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const float& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const double& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}
void Log(const Mat4x4& out) {
#ifdef _DEBUG
	DebugLog(out)
#endif // _DEBUG
}

void FsReadDiskFile(std::string* content, const std::string& filePath) {
	std::ifstream fileStream(filePath, std::ios::in);
	if(!fileStream.is_open()) return;
	std::string line;
	while(std::getline(fileStream, line)) {
		*content+=line+"\n";
	}
	fileStream.close();
}

Mat4x4::Mat4x4(float _values[16]) { for(int i=0; i<16; i++) { values[i]=_values[i]; } }
Mat4x4::Mat4x4() { float _values[16]={ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; for(int i=0; i<16; i++) { values[i]=_values[i]; } isIdentity=true; }
float Mat4x4::get(const int& x, const int& y) const { return values[y*4+x]; }
float Mat4x4::operator[](const int& i) const { return values[i]; }
Vector4 Mat4x4::getColumn(const int& x) const { return Vector4(get(x, 0), get(x, 1), get(x, 2), get(x, 3)); }
Vector4 Mat4x4::getRow(const int& y) const { return Vector4(get(0, y), get(1, y), get(2, y), get(3, y)); }
Vector4 Mat4x4::operator*(const Vector4& b) const {
	return Vector4(
		getRow(0).dot(b),
		getRow(1).dot(b),
		getRow(2).dot(b),
		getRow(3).dot(b)
	);
}
Vector3 Mat4x4::operator*(const Vector3& b) const {
	return operator*(Vector4(b, 1.0f)).XYZ();
}
Mat4x4 Mat4x4::operator*(const Mat4x4& b) const {
	if(b.isIdentity) return *this;
	else if(isIdentity) return b;
	float ary[16]={
		getRow(0).dot(b.getColumn(0)), getRow(0).dot(b.getColumn(1)), getRow(0).dot(b.getColumn(2)), getRow(0).dot(b.getColumn(3)),
		getRow(1).dot(b.getColumn(0)), getRow(1).dot(b.getColumn(1)), getRow(1).dot(b.getColumn(2)), getRow(1).dot(b.getColumn(3)),
		getRow(2).dot(b.getColumn(0)), getRow(2).dot(b.getColumn(1)), getRow(2).dot(b.getColumn(2)), getRow(2).dot(b.getColumn(3)),
		getRow(3).dot(b.getColumn(0)), getRow(3).dot(b.getColumn(1)), getRow(3).dot(b.getColumn(2)), getRow(3).dot(b.getColumn(3))
	};
	return Mat4x4(ary);
}
Mat4x4::operator std::string() const {
	return "[ "+std::to_string(get(0, 0))+", "+std::to_string(get(1, 0))+", "+std::to_string(get(2, 0))+", "+std::to_string(get(3, 0))+","+
		"\n  "+std::to_string(get(0, 1))+", "+std::to_string(get(1, 1))+", "+std::to_string(get(2, 1))+", "+std::to_string(get(3, 1))+","+
		"\n  "+std::to_string(get(0, 2))+", "+std::to_string(get(1, 2))+", "+std::to_string(get(2, 2))+", "+std::to_string(get(3, 2))+","+
		"\n  "+std::to_string(get(0, 3))+", "+std::to_string(get(1, 3))+", "+std::to_string(get(2, 3))+", "+std::to_string(get(3, 3))+"]";
}
#define PI 3.1415926535897932384626433832795f
float deg_to_rad(const float& deg) {
	return deg*PI/180.0f;
}
float rad_to_deg(const float& rad) {
	return rad/PI*180.0f;
}
Mat4x4 translate(const Vector3& value) {
	if(value==Vector3::ZERO) return Mat4x4();//returns identity matrix
	float ary[16]={
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		value.x, value.y, value.z, 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 scaleMat(const Vector3& value) {
	if(value==Vector3::ONE) return Mat4x4();//returns identity matrix
	float ary[16]={
		value.x, 0.0f, 0.0f, 0.0f,
		0.0f, value.y, 0.0f, 0.0f,
		0.0f, 0.0f, value.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 axisRotMat(const Vector3& axis, const float& angle) {
	if(angle==0.0f||axis==Vector3::ZERO) return Mat4x4();//returns identity matrix
	float c=cosf(angle);
	float s=sinf(angle);
	Vector3 a=axis.normalized();
	Vector3 temp=a*(1-c);
	float Rotate[16]={
		c+temp.x*a.x, temp.x*a.y+s*a.z, temp.x*a.z-s*a.y, 0,
		temp.y*a.x-s*a.z, c+temp.y*a.y, temp.y*a.z+s*a.x, 0,
		temp.z*a.x+s*a.y, temp.z*a.y-s*a.x, c+temp.z*a.z, 0,
		0, 0, 0, 1
	};
	return Mat4x4(Rotate);
}
Mat4x4 ortho(const float& left, const float& right, const float& bottom, const float& top, const float& near, const float& far) {
	float oneoverhorizontal=1/(right-left);
	float oneoververtical=1/(top-bottom);
	float oneoverfarnear=1/(far-near);
	float ary[16]={
		oneoverhorizontal*2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, oneoververtical*2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -oneoverfarnear*2.0f, 0.0f,
		-(right+left)*oneoverhorizontal, -(top+bottom)*oneoververtical, -(far+near)*oneoverfarnear, 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 perspective(const float& fov, const float& aspect, const float& near, const float& far) {
	float oneoverhalftan=1.0f/tanf(tan(fov/2.0f));
	float ary[16]={
		oneoverhalftan/aspect, 0.0f, 0.0f, 0.0f,
		0.0f, oneoverhalftan, 0.0f, 0.0f,
		0.0f, 0.0f, far/(near-far), -1.0f,
		0.0f, 0.0f, -far*near/(far-near), 0.0f
	};
	return Mat4x4(ary);
}
Mat4x4 lookAt(const Vector3& position, const Vector3& target, const Vector3& up) {
	Vector3 F=(target-position).normalized();// direction
	Vector3 S=F.cross(up).normalized();// right vector
	Vector3 U=S.cross(F).normalized();// right vector
	float ary[16]={
		S.x, U.x, -F.x, 0.0f,
		S.y, U.y, -F.y, 0.0f,
		S.z, U.z, -F.z, 0.0f,
		-S.dot(position), -U.dot(position), F.dot(position), 1.0f
	};
	return translate(-position)*Mat4x4(ary);
}