#include "Lib.h"
void Log(string out) {
#ifdef _DEBUG
	cout << out << endl;
#endif // _DEBUG
}
void Log(const char* out) { Log(string(out)); }
void Log(bool out) { Log(out?"True":"False"); }
void Log(int out) { Log(to_string(out)); }
void Log(unsigned int out) { Log(to_string(out)); }
void Log(float out) { Log(to_string(out)); }
void Log(double out) { Log(to_string(out)); }
void Log(Vector2 out) { Log("(" + to_string(out.x) + ", " + to_string(out.y) + ")"); }
void Log(Vector3 out) { Log("(" + to_string(out.x) + ", " + to_string(out.y) + ", " + to_string(out.z) + ")"); }
void Log(Mat4x4 out) { Log(out.to_string()); }

Vector2::Vector2() { x=0; y=0; }
Vector2::Vector2(float _x, float _y) { x=_x; y=_y; }
Vector2::Vector2(float v) { x=v; y=v; }// from a single value
void Vector2::operator+=(Vector2 b) { x+=b.x; y+=b.y; }
void Vector2::operator-=(Vector2 b) { x-=b.x; y-=b.y; }
float Vector2::length() { return sqrt(x * x + y * y); }
float Vector2::dot(Vector2 b) { return (x * b.x) + (y * b.y); }
Vector2 Vector2::normalized() {
	float len=length();
	return len != 0?(Vector2(x, y) / len):Vector2(x, y);
}
Vector2 Vector2::operator+(Vector2 b) { return Vector2(x + b.x, y + b.y); }
Vector2 Vector2::operator-(Vector2 b) { return Vector2(x - b.x, y - b.y); }
Vector2 Vector2::operator-() { return Vector2(-x, -y); }
Vector2 Vector2::operator*(float b) { return Vector2(x * b, y * b); }
Vector2 Vector2::operator*(int b) { return Vector2(x * b, y * b); }
Vector2 Vector2::operator/(float b) { return Vector2(x / b, y / b); }
Vector2 Vector2::operator/(int b) { return Vector2(x / b, y / b); }
std::string Vector2::to_string() {
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

Vector3::Vector3() { x=0; y=0; z=0; }
Vector3::Vector3(Vector2 v, float _z) { x=v.x; y=v.y; z=_z; }
Vector3::Vector3(float _x, float _y, float _z) { x=_x; y=_y; z=_z; }
Vector3::Vector3(float v) { x=v; y=v; z=v; }// from a single value
Vector3 Vector3::cross(Vector3 b) {
	return Vector3(
		y * b.z - z * b.y,
		z * b.x - x * b.z,
		x * b.y - y * b.x
	);
}
void Vector3::operator+=(Vector3 b) { x+=b.x; y+=b.y; z+=b.z; }
void Vector3::operator-=(Vector3 b) { x-=b.x; y-=b.y; z-=b.z; }
float Vector3::length() { return sqrt(x * x + y * y + z * z); }
float Vector3::dot(Vector3 b) { return (x * b.x) + (y * b.y) + (z * b.z); }
Vector3 Vector3::normalized() {
	float len=length();
	return len != 0?(Vector3(x, y, z) / len):Vector3(x, y, z);
}
Vector3 Vector3::operator+(Vector3 b) { return Vector3(x + b.x, y + b.y, z + b.z); }
Vector3 Vector3::operator-(Vector3 b) { return Vector3(x - b.x, y - b.y, z - b.z); }
Vector3 Vector3::operator-() { return Vector3(-x, -y, -z); }
Vector3 Vector3::operator*(float b) { return Vector3(x * b, y * b, z * b); }
Vector3 Vector3::operator*(int b) { return Vector3(x * b, y * b, z * b); }
Vector3 Vector3::operator/(float b) { return Vector3(x / b, y / b, z / b); }
Vector3 Vector3::operator/(int b) { return Vector3(x / b, y / b, z / b); }
std::string Vector3::to_string() {
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
}

Vector4::Vector4() { x=0; y=0; z=0; w=0; }
Vector4::Vector4(Vector3 v, float _w) { x=v.x; y=v.y; z=v.z; w=_w; }
Vector4::Vector4(float _x, float _y, float _z, float _w) { x=_x; y=_y; z=_z; w=_w; }
Vector4::Vector4(float v) { x=v; y=v; z=v; w=v; }// from a single value
Vector3 Vector4::toXYZ() { return Vector3(x, y, z); }
void Vector4::operator+=(Vector4 b) { x+=b.x; y+=b.y; z+=b.z; w+=b.w; }
void Vector4::operator-=(Vector4 b) { x-=b.x; y-=b.y; z-=b.z; w-=b.w; }
float Vector4::length() { return sqrt(x * x + y * y + z * z + w * w); }
float Vector4::dot(Vector4 b) { return (x * b.x) + (y * b.y) + (z * b.z) + (w * b.w); }
Vector4 Vector4::normalized() {
	float len=length();
	return len != 0?(Vector4(x, y, z, w) / len):Vector4(x, y, z, w);
}
Vector4 Vector4::operator+(Vector4 b) { return Vector4(x + b.x, y + b.y, z + b.z, w + b.w); }
Vector4 Vector4::operator-(Vector4 b) { return Vector4(x - b.x, y - b.y, z - b.z, w - b.w); }
Vector4 Vector4::operator-() { return Vector4(-x, -y, -z, -w); }
Vector4 Vector4::operator*(float b) { return Vector4(x * b, y * b, z * b, w * b); }
Vector4 Vector4::operator*(int b) { return Vector4(x * b, y * b, z * b, w * b); }
Vector4 Vector4::operator/(float b) { return Vector4(x / b, y / b, z / b, w / b); }
Vector4 Vector4::operator/(int b) { return Vector4(x / b, y / b, z / b, w / b); }
std::string Vector4::to_string() {
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ")";
}

Mat4x4::Mat4x4() { float _values[16]={ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; for(int i=0; i < 16; i++) { values[i]=_values[i]; } }
Mat4x4::Mat4x4(float _values[16]) { for(int i=0; i < 16; i++) { values[i]=_values[i]; } }
float Mat4x4::get(int x, int y) { return values[y * 4 + x]; }
float Mat4x4::operator[](int i) { return values[i]; }
Vector4 Mat4x4::getColumn(int x) { return Vector4(get(x, 0), get(x, 1), get(x, 2), get(x, 3)); }
Vector4 Mat4x4::getRow(int y) { return Vector4(get(0, y), get(1, y), get(2, y), get(3, y)); }
Vector4 Mat4x4::operator*(Vector4 b) {
	return Vector4(
		getRow(0).dot(b),
		getRow(1).dot(b),
		getRow(2).dot(b),
		getRow(3).dot(b)
	);
}
Mat4x4 Mat4x4::operator*(Mat4x4 b) {
	float ary[16]={
		getRow(0).dot(b.getColumn(0)), getRow(0).dot(b.getColumn(1)), getRow(0).dot(b.getColumn(2)), getRow(0).dot(b.getColumn(3)),
		getRow(1).dot(b.getColumn(0)), getRow(1).dot(b.getColumn(1)), getRow(1).dot(b.getColumn(2)), getRow(1).dot(b.getColumn(3)),
		getRow(2).dot(b.getColumn(0)), getRow(2).dot(b.getColumn(1)), getRow(2).dot(b.getColumn(2)), getRow(2).dot(b.getColumn(3)),
		getRow(3).dot(b.getColumn(0)), getRow(3).dot(b.getColumn(1)), getRow(3).dot(b.getColumn(2)), getRow(3).dot(b.getColumn(3))
	};
	return Mat4x4(ary);
}
std::string Mat4x4::to_string() {
	return "[ " + std::to_string(get(0, 0)) + ", " + std::to_string(get(1, 0)) + ", " + std::to_string(get(2, 0)) + ", " + std::to_string(get(3, 0)) + "," +
		"\n  " + std::to_string(get(0, 1)) + ", " + std::to_string(get(1, 1)) + ", " + std::to_string(get(2, 1)) + ", " + std::to_string(get(3, 1)) + "," +
		"\n  " + std::to_string(get(0, 2)) + ", " + std::to_string(get(1, 2)) + ", " + std::to_string(get(2, 2)) + ", " + std::to_string(get(3, 2)) + "," +
		"\n  " + std::to_string(get(0, 3)) + ", " + std::to_string(get(1, 3)) + ", " + std::to_string(get(2, 3)) + ", " + std::to_string(get(3, 3)) + "]";
}
#define PI 3.1415926535897932384626433832795f
float deg_to_rad(float deg) {
	return deg * PI / 180.0f;
}
float rad_to_deg(float rad) {
	return rad / PI * 180.0f;
}
Mat4x4 translate(Vector3 value) {
	float ary[16]={
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		value.x, value.y, value.z, 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 scaleMat(Vector3 value) {
	float ary[16]={
		value.x, 0.0f, 0.0f, 0.0f,
		0.0f, value.y, 0.0f, 0.0f,
		0.0f, 0.0f, value.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 axisRotMat(Vector3 axis, float angle) {
	float c=cos(angle);
	float s=sin(angle);
	Vector3 a=axis.normalized();
	Vector3 temp=a * (1 - c);
	float Rotate[16]={
		c + temp.x * a.x, temp.x * a.y + s * a.z, temp.x * a.z - s * a.y, 0,
		temp.y * a.x - s * a.z, c + temp.y * a.y, temp.y * a.z + s * a.x, 0,
		temp.z * a.x + s * a.y, temp.z * a.y - s * a.x, c + temp.z * a.z, 0,
		0, 0, 0, 1
	};
	return Mat4x4(Rotate);
}
Mat4x4 ortho(float left, float right, float bottom, float top, float near, float far) {
	float ary[16]={
		2.0f / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f / (far - near), 0.0f,
		-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
	};
	return Mat4x4(ary);
}
Mat4x4 perspective(float fov, float aspect, float near, float far) {
	float ary[16]={
		1.0f / (aspect * tan(fov / 2.0f)), 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f / tan(fov / 2.0f), 0.0f, 0.0f,
		0.0f, 0.0f, far / (near - far), -1.0f,
		0.0f, 0.0f, -far * near / (far - near), 0.0f
	};
	return Mat4x4(ary);
}
Mat4x4 lookAt(Vector3 position, Vector3 target, Vector3 up) {
	Vector3 F=(target - position).normalized();// direction
	Vector3 S=F.cross(up).normalized();// right vector
	Vector3 U=S.cross(F).normalized();// right vector
	float ary[16]={
		S.x, U.x, -F.x, 0.0f,
		S.y, U.y, -F.y, 0.0f,
		S.z, U.z, -F.z, 0.0f,
		-S.dot(position), -U.dot(position), F.dot(position), 1.0f
	};
	return translate(-position) * Mat4x4(ary);
}