#include "EngineLib.h"
void Log(const std::string& out) {
#ifdef _DEBUG
	std::cout<<out<<std::endl;
#endif // _DEBUG
}
void Log(const char* out) {
#ifdef _DEBUG
	std::cout<<out<<std::endl;
#endif // _DEBUG
}
void Log(const bool& out) { Log(out ? "True" : "False"); }
void Log(const int& out) { Log(std::to_string(out)); }
void Log(const unsigned int& out) { Log(std::to_string(out)); }
void Log(const float& out) { Log(std::to_string(out)); }
void Log(const double& out) { Log(std::to_string(out)); }
void Log(const Vector2& out) { Log(out.to_string()); }
void Log(const Vector3& out) { Log(out.to_string()); }
void Log(const Vector4& out) { Log(out.to_string()); }
void Log(const Mat4x4& out) { Log(out.to_string()); }

void FsReadDiskFile(std::string* content, const std::string& filePath) {
	std::ifstream fileStream(filePath, std::ios::in);
	if(!fileStream.is_open()) return;
	std::string line;
	while(std::getline(fileStream, line)) {
		*content+=line+"\n";
	}
	fileStream.close();
}

Vector2::Vector2() { x=0; y=0; }
Vector2::Vector2(const float& _x, const float& _y) { x=_x; y=_y; }
Vector2::Vector2(const float& v) { x=v; y=v; }// from a single value
void Vector2::operator+=(const Vector2& b) { x+=b.x; y+=b.y; }
void Vector2::operator-=(const Vector2& b) { x-=b.x; y-=b.y; }
float Vector2::length() const { return sqrt(x*x+y*y); }
float Vector2::cross(const Vector2& b) const {
	return x*b.y-y*b.x;
}
float Vector2::dot(const Vector2& b) const { return (x*b.x)+(y*b.y); }
Vector2 Vector2::normalized() const {
	float len=length();
	return len!=0 ? (Vector2(x, y)/len) : Vector2(x, y);
}
Vector2 Vector2::abs() const { return Vector2(std::abs(x), std::abs(y)); }
Vector2 Vector2::floor() const { return Vector2(std::floor(x), std::floor(y)); }
Vector2 Vector2::round() const { return Vector2(std::round(x), std::round(y)); }
Vector2 Vector2::ceil() const { return Vector2(std::ceil(x), std::ceil(y)); }
Vector2 Vector2::operator+(const Vector2& b) const { return Vector2(x+b.x, y+b.y); }
Vector2 Vector2::operator-(const Vector2& b) const { return Vector2(x-b.x, y-b.y); }
Vector2 Vector2::operator-() const { return Vector2(-x, -y); }
Vector2 Vector2::operator*(const float& b) const { return Vector2(x*b, y*b); }
Vector2 Vector2::operator*(const double& b) const { return Vector2(x*((float)b), y*((float)b)); }
Vector2 Vector2::operator*(const int& b) const { return Vector2(x*((float)b), y*((float)b)); }
Vector2 Vector2::operator/(const float& b) const { return Vector2(x/b, y/b); }
Vector2 Vector2::operator/(const double& b) const { return Vector2(x/((float)b), y/((float)b)); }
Vector2 Vector2::operator/(const int& b) const { return Vector2(x/((float)b), y/((float)b)); }
float Vector2::operator[](const int& i) const {
	if(i==0) return x;
	else if(i==1) return y;
	else { Log("Vector2: Index out of range");exit(0); }
}
bool Vector2::operator==(const Vector2& b) const { return (x==b.x)&&(y==b.y); }
bool Vector2::operator!=(const Vector2& b) const { return !(operator==(b)); }
std::string Vector2::to_string() const {
	return "("+std::to_string(x)+", "+std::to_string(y)+")";
}
Vector2::operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+")"; }
const Vector2 Vector2::ZERO=Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONE=Vector2(1.0f, 1.0f);
const Vector2 Vector2::RIGHT=Vector2(1.0f, 0.0f);
const Vector2 Vector2::UP=Vector2(0.0f, 1.0f);
const Vector2 Vector2::Center=Vector2(0.0f, 0.0f);
const Vector2 Vector2::TopRight=Vector2(0.5f, 0.5f);
const Vector2 Vector2::TopLeft=Vector2(-0.5f, 0.5f);
const Vector2 Vector2::BottomLeft=Vector2(-0.5f, -0.5f);
const Vector2 Vector2::BottomRight=Vector2(0.5f, -0.5f);

Vector3::Vector3() { x=0; y=0; z=0; }
Vector3::Vector3(const Vector2& v, const float& _z) { x=v.x; y=v.y; z=_z; }
Vector3::Vector3(const float& _x, const float& _y, const float& _z) { x=_x; y=_y; z=_z; }
Vector3::Vector3(const float& v) { x=v; y=v; z=v; }// from a single value
Vector2 Vector3::toXY() const { return Vector2(x, y); }
void Vector3::operator+=(const Vector3& b) { x+=b.x; y+=b.y; z+=b.z; }
void Vector3::operator-=(const Vector3& b) { x-=b.x; y-=b.y; z-=b.z; }
float Vector3::length() const { return sqrt(x*x+y*y+z*z); }
Vector3 Vector3::cross(const Vector3& b) const {
	return Vector3(
		y*b.z-z*b.y,
		z*b.x-x*b.z,
		x*b.y-y*b.x
	);
}
float Vector3::dot(const Vector3& b) const { return (x*b.x)+(y*b.y)+(z*b.z); }
Vector3 Vector3::floor() const { return Vector3(std::floor(x), std::floor(y), std::floor(z)); }
Vector3 Vector3::round() const { return Vector3(std::round(x), std::round(y), std::round(z)); }
Vector3 Vector3::ceil() const { return Vector3(std::ceil(x), std::ceil(y), std::ceil(z)); }
Vector3 Vector3::normalized() const {
	float len=length();
	return len!=0 ? (Vector3(x, y, z)/len) : Vector3(x, y, z);
}
Vector3 Vector3::abs() const { return Vector3(std::abs(x), std::abs(y), std::abs(z)); }
Vector3 Vector3::operator+(const Vector3& b) { return Vector3(x+b.x, y+b.y, z+b.z); }
Vector3 Vector3::operator-(const Vector3& b) const { return Vector3(x-b.x, y-b.y, z-b.z); }
Vector3 Vector3::operator-() const { return Vector3(-x, -y, -z); }
Vector3 Vector3::operator*(const float& b) const { return Vector3(x*b, y*b, z*b); }
Vector3 Vector3::operator*(const double& b) const { return Vector3(x*((float)b), y*((float)b), z*((float)b)); }
Vector3 Vector3::operator*(const int& b) const { return Vector3(x*((float)b), y*((float)b), z*((float)b)); }
Vector3 Vector3::operator/(const float& b) const { return Vector3(x/b, y/b, z/b); }
Vector3 Vector3::operator/(const double& b) const { return Vector3(x/((float)b), y/((float)b), z/((float)b)); }
Vector3 Vector3::operator/(const int& b) const { return Vector3(x/((float)b), y/((float)b), z/((float)b)); }
float Vector3::operator[](const int& i) const {
	if(i==0) return x;
	else if(i==1) return y;
	else if(i==2) return z;
	else { Log("Vector3: Index out of range");exit(0); }
}
bool Vector3::operator==(const Vector3& b) const { return (x==b.x)&&(y==b.y)&&(z==b.z); }
bool Vector3::operator!=(const Vector3& b) const { return !(*this==b); }
std::string Vector3::to_string() const {
	return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";
}
Vector3::operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")"; }
const Vector3 Vector3::ZERO=Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::ONE=Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::RIGHT=Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UP=Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::FORWARD=Vector3(0.0f, 0.0f, 1.0f);

Vector4::Vector4() { x=0; y=0; z=0; w=0; }
Vector4::Vector4(const Vector3& v, const float& _w) { x=v.x; y=v.y; z=v.z; w=_w; }
Vector4::Vector4(const float& _x, const float& _y, const float& _z, const float& _w) { x=_x; y=_y; z=_z; w=_w; }
Vector4::Vector4(const float& v) { x=v; y=v; z=v; w=v; }// from a single value
Vector3 Vector4::toXYZ() const { return Vector3(x, y, z); }
void Vector4::operator+=(const Vector4& b) { x+=b.x; y+=b.y; z+=b.z; w+=b.w; }
void Vector4::operator-=(const Vector4& b) { x-=b.x; y-=b.y; z-=b.z; w-=b.w; }
float Vector4::length() const { return sqrt(x*x+y*y+z*z+w*w); }
float Vector4::dot(const Vector4& b) const { return (x*b.x)+(y*b.y)+(z*b.z)+(w*b.w); }
Vector4 Vector4::normalized() const {
	float len=length();
	return len!=0 ? (Vector4(x, y, z, w)/len) : Vector4(x, y, z, w);
}
Vector4 Vector4::abs() const { return Vector4(std::abs(x), std::abs(y), std::abs(z), std::abs(w)); }
Vector4 Vector4::floor() const { return Vector4(std::floor(x), std::floor(y), std::floor(z), std::floor(w)); }
Vector4 Vector4::round() const { return Vector4(std::round(x), std::round(y), std::round(z), std::round(w)); }
Vector4 Vector4::ceil() const { return Vector4(std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w)); }
Vector4 Vector4::operator+(const Vector4& b) const { return Vector4(x+b.x, y+b.y, z+b.z, w+b.w); }
Vector4 Vector4::operator-(const Vector4& b) const { return Vector4(x-b.x, y-b.y, z-b.z, w-b.w); }
Vector4 Vector4::operator-() const { return Vector4(-x, -y, -z, -w); }
Vector4 Vector4::operator*(const float& b) const { return Vector4(x*b, y*b, z*b, w*b); }
Vector4 Vector4::operator*(const double& b) const { return Vector4(x*((float)b), y*((float)b), z*((float)b), w*((float)b)); }
Vector4 Vector4::operator*(const int& b) const { return Vector4(x*((float)b), y*((float)b), z*((float)b), w*((float)b)); }
Vector4 Vector4::operator/(const float& b) const { return Vector4(x/b, y/b, z/b, w/b); }
Vector4 Vector4::operator/(const double& b) const { return Vector4(x/((float)b), y/((float)b), z/((float)b), w/((float)b)); }
Vector4 Vector4::operator/(const int& b) const { return Vector4(x/((float)b), y/((float)b), z/((float)b), w/((float)b)); }
float Vector4::operator[](const int& i) const {
	if(i==0) return x;
	else if(i==1) return y;
	else if(i==2) return z;
	else if(i==3) return w;
	else { Log("Vector4: Index out of range");exit(0); }
}
bool Vector4::operator==(const Vector4& b) const { return (x==b.x)&&(y==b.y)&&(z==b.z)&&(w==b.w); }
bool Vector4::operator!=(const Vector4& b) const { return !(*this==b); }
std::string Vector4::to_string() const {
	return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";
}
Vector4::operator std::string() const { return "("+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")"; }
const Vector4 Vector4::ZERO=Vector4(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::ONE=Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Vector4::RIGHT=Vector4(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UP=Vector4(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::FORWARD=Vector4(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::ANA=Vector4(0.0f, 0.0f, 0.0f, 1.0f);

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
	return operator*(Vector4(b, 1.0f)).toXYZ();
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
std::string Mat4x4::to_string() const {
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