//
// Created by School on 2025/3/11.
//

#include <data_type.h>
#include <format>


#pragma region Vector2
#pragma region Vector2<float> methods
#pragma region Constructors
Vector2<float>::Vector2(): x(0), y(0){}

Vector2<float>::Vector2(float val): x(val), y(val) {}

Vector2<float>::Vector2(const Vector2<int>& other): x(static_cast<float>(other.x)), y(static_cast<float>(other.y)){}

Vector2<float>::Vector2(float x, float y): x(x), y(y) {}
#pragma endregion

#pragma region Overrides
Vector2<float> Vector2<float>::operator/(int scalar) const {
	return {static_cast<float>(x) / static_cast<float>(scalar), static_cast<float>(x) / static_cast<float>(scalar)};
}

Vector2<float> Vector2<float>::operator*(int scalar) const {
	return {static_cast<float>(x) * static_cast<float>(scalar), static_cast<float>(x) * static_cast<float>(scalar)};
}
std::ostream& operator<<(std::ostream& out, const Vector2<float> vector2)  {
	out << '(' << vector2.x << ',' << vector2.y << ')';
	return out;
}
#pragma endregion
#pragma endregion

#pragma region Vector<float> Constants
const Vector2<float> Vector2<float>::ZERO{};

const Vector2<float> Vector2<float>::ONE{1};

const Vector2<float> Vector2<float>::UP{0, -1};

const Vector2<float> Vector2<float>::DOWN{0, 1};

const Vector2<float> Vector2<float>::LEFT{-1, 0};

const Vector2<float> Vector2<float>::RIGHT{1, 0};
#pragma endregion

#pragma region Vector2<int> methods
#pragma region Constructors
Vector2<int>::Vector2() : x(0), y(0) {}

Vector2<int>::Vector2(int val) : x(val), y(val) {}

Vector2<int>::Vector2(POINT point) : x(static_cast<int>(point.x)), y(static_cast<int>(point.y)){}

Vector2<int>::Vector2(const Vector2<float>& other) : x(static_cast<int>(other.x)), y(static_cast<int>(other.y)){}

Vector2<int>::Vector2(int x, int y) : x(x), y(y) {}
#pragma endregion

#pragma region Overrides
Vector2<float> Vector2<int>::operator*(float scalar) const {
	return {static_cast<float>(x) * scalar, static_cast<float>(x) * scalar};
}

Vector2<float> Vector2<int>::operator*(const Vector2<float>& other) const  {
	return {static_cast<float>(x) * other.x, static_cast<float>(x) * other.y};
}

Vector2<float> Vector2<int>::operator/(float scalar) const   {
	return {static_cast<float>(x) / scalar, static_cast<float>(x) / scalar};
}

Vector2<float> Vector2<int>::operator/(const Vector2<float>& other) const  {
	return {static_cast<float>(x) / other.x, static_cast<float>(x) / other.y};
}

std::ostream& operator<<(std::ostream& out, const Vector2<int> vector2)  {
	out << '(' << vector2.x << ',' << vector2.y << ')';
	return out;
}
#pragma endregion
#pragma endregion

#pragma region Vector<int> Constants
const Vector2<int> Vector2<int>::ZERO{};

const Vector2<int> Vector2<int>::ONE{1};

const Vector2<int> Vector2<int>::UP{0, -1};

const Vector2<int> Vector2<int>::DOWN{0, 1};

const Vector2<int> Vector2<int>::LEFT{-1, 0};

const Vector2<int> Vector2<int>::RIGHT{1, 0};
#pragma endregion
#pragma endregion

#pragma region Rect2
#pragma region Rect2<float> methods
Rect2<float>::Rect2() : pos(), size(){}

Rect2<float>::Rect2(Vector2<float> pos, Vector2<float> size) : pos(pos), size(size){}

Rect2<float>::Rect2(float x, float y, float width, float height) : pos(Vector2<float>(x, y)), size(Vector2<float>(width, height)){}

Rect2<float>::Rect2(const Rect2<int>& other) : pos(static_cast<Vector2<float>>(other.pos)), size(static_cast<Vector2<float>>(other.size)) {}
#pragma endregion

#pragma region Rect2<int> methods
Rect2<int>::Rect2() : pos(), size(){}

Rect2<int>::Rect2(RECT rect): pos(Vector2<int>(rect.left, rect.top)), size(Vector2<int>(rect.right - rect.left, rect.bottom - rect.top)) {}

Rect2<int>::Rect2(Vector2<int> pos, Vector2<int> size) : pos(pos), size(size){}

Rect2<int>::Rect2(int x, int y, int width, int height) : pos(Vector2<int>(x, y)), size(Vector2<int>(width, height)){}

Rect2<int>::Rect2(const Rect2<float> &other) : pos(static_cast<Vector2<int>>(other.pos)), size(static_cast<Vector2<int>>(other.size)) {}
#pragma endregion
#pragma endregion
#pragma region Colour
Colour::Colour(): r(0), g(0), b(0), a(0) {}
Colour::Colour(unsigned char v): r(v), g(v), b(v), a(v) {}
Colour::Colour(unsigned char r, unsigned char g, unsigned char b, unsigned char a):
	r(r), g(g), b(b), a(a){}
Colour Colour::fromFloat(float r, float g, float b, float a){
    return Colour(
        static_cast<unsigned char>(r * 255),
	    static_cast<unsigned char>(g * 255),
	    static_cast<unsigned char>(b * 255),
	    static_cast<unsigned char>(a * 255)
    );
}
Colour::Colour(std::string hex) {
	if(hex.at(0) == '#') {
		hex.erase(0, 1);
	}
	if (hex.length() != 6 && hex.length() != 8) {
		throw std::invalid_argument("hex string should either be 6 or 8 characters long, optionally prepended with a hashtag");
	}
	r = static_cast<unsigned char>(std::stoi(hex.substr(0, 2), nullptr, 16));
	g = static_cast<unsigned char>(std::stoi(hex.substr(0, 2), nullptr, 16));
	b = static_cast<unsigned char>(std::stoi(hex.substr(0, 2), nullptr, 16));
	if (hex.length() == 8) {
		a = static_cast<unsigned char>(std::stoi(hex.substr(0, 2), nullptr, 16));
	} else {
		a = 255;
	}
}

std::string Colour::getHex(bool with_alpha) {
	std::string hex = std::format("{:x}", r) + std::format("{:x}", g) + std::format("{:x}", b);
	if (with_alpha) {
		hex += std::format("{:x}", a);
	}
	return hex;
}
#pragma region Constants
const Colour Colour::NONE{0,0,0,0};
const Colour Colour::WHITE{255,255,255,255};
const Colour Colour::BLACK{0,0,0,255};
#pragma endregion
#pragma endregion
