#include "Vec2.h"
#include <math.h>

Vec2::Vec2() {}

Vec2::Vec2(float xin, float yin) 
	: x(xin) , y(yin) {}

bool Vec2::operator == (const Vec2& rhs) const {
	return (x == rhs.x) && (y == rhs.y); 
}
bool Vec2::operator != (const Vec2& rhs) const {
	return (x != rhs.x) || (y != rhs.y); 
}

Vec2 Vec2::operator + (const Vec2& rhs) const {
	return Vec2(x + rhs.x, y + rhs.y); 
}
Vec2 Vec2::operator - (const Vec2& rhs) const {
	return Vec2(x - rhs.x, y - rhs.y);
}
Vec2 Vec2::operator / (const float val) const { // divide with scalar number
	return Vec2(x / val, y / val);
}
Vec2 Vec2::operator * (const float val) const {
	return Vec2(x * val, y * val);
}

void Vec2::operator += (const Vec2& rhs) { // not const because want to modify x , y
	x += rhs.x, y += rhs.y; 
}
void Vec2::operator -= (const Vec2& rhs) {
	x -= rhs.x, y -= rhs.y; 
}
void Vec2::operator *= (const float val) {
	x *= val , y *= val; 
}
void Vec2::operator /= (const float val) {
	x /= val , y /= val; 
}

float Vec2::dist(const Vec2& rhs) const {
	return sqrt(pow(rhs.x - x, 2) + pow(rhs.y - y, 2)); 
}

void Vec2::normalize() {
	float L = sqrt(x * x + y * y);
	x /= L;
	y /= L;
}