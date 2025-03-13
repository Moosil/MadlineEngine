//
// Created by School on 2025/3/11.
//

#include <data_type.h>
#include <format>

glm::vec<2, int> vec2iFromPoint(POINT point) {
	return {point.x, point.y};
}

#pragma region Rect2
#pragma region Rect2<float> methods
Rect2<float>::Rect2() : pos(), size(){}

Rect2<float>::Rect2(glm::vec2 pos, glm::vec2 size) : pos(pos), size(size){}

Rect2<float>::Rect2(float x, float y, float width, float height) : pos(glm::vec2(x, y)), size(glm::vec2(width, height)){}

Rect2<float>::Rect2(const Rect2<int>& other) : pos(static_cast<glm::vec2>(other.pos)), size(static_cast<glm::vec2>(other.size)) {}
#pragma endregion

#pragma region Rect2<int> methods
Rect2<int>::Rect2() : pos(), size(){}

Rect2<int>::Rect2(RECT rect): pos(glm::vec<2, int>(rect.left, rect.top)), size(glm::vec<2, int>(rect.right - rect.left, rect.bottom - rect.top)) {}

Rect2<int>::Rect2(glm::vec<2, int> pos, glm::vec<2, int> size) : pos(pos), size(size){}

Rect2<int>::Rect2(int x, int y, int width, int height) : pos(glm::vec<2, int>(x, y)), size(glm::vec<2, int>(width, height)){}

Rect2<int>::Rect2(const Rect2<float> &other) : pos(static_cast<glm::vec<2, int>>(other.pos)), size(static_cast<glm::vec<2, int>>(other.size)) {}
#pragma endregion
#pragma endregion
#pragma region Colour
Colour::Colour(): r(0), g(0), b(0), a(0) {}
Colour::Colour(unsigned char v): r(v), g(v), b(v), a(v) {}
Colour::Colour(unsigned char r, unsigned char g, unsigned char b, unsigned char a):
	r(r), g(g), b(b), a(a){}
Colour Colour::fromFloat(float r, float g, float b, float a){
    return {
        static_cast<unsigned char>(r * 255),
	    static_cast<unsigned char>(g * 255),
	    static_cast<unsigned char>(b * 255),
	    static_cast<unsigned char>(a * 255)
    };
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
