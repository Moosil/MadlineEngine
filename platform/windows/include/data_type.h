//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_DATA_TYPE_H
#define CELESTEPET_DATA_TYPE_H

#include <ostream>
#include <windows.h>
#include <glm/glm.hpp>

glm::vec<2, int> vec2iFromPoint(POINT point);

#pragma region Rect
template<typename T>
class Rect2 {
public:
	glm::vec<2, T> pos, size;

	Rect2() : pos(), size(){};

	Rect2(const Rect2 &other) = default;

	Rect2(glm::vec<2, T> pos, glm::vec<2, T> size) : pos(pos), size(size){};

	Rect2(T x, T y, T width, T height) : pos(Vector2<T>(x, y)), size(Vector2<T>(width, height)){};

	bool operator==(const Rect2 &other) const {
		return pos == other.pos && size == other.size;
	}

	bool operator!=(const Rect2 &other) const {
		return pos != other.pos || size != other.size;
	}
};

template<>
class Rect2<float> {
public:
	glm::vec2 pos, size;
	
	Rect2();
	
	Rect2(const Rect2 &other) = default;
	
	explicit Rect2(const Rect2<int> &other);

	Rect2(glm::vec2 pos, glm::vec2 size);

	Rect2(float x, float y, float width, float height);
};

template<>
class Rect2<int> {
public:
	glm::vec<2, int> pos, size;

	Rect2();
	
	Rect2(RECT rect);
	
	Rect2(const Rect2 &other) = default;
	
	explicit Rect2(const Rect2<float> &other);
	
	Rect2(glm::vec<2, int> pos, glm::vec<2, int> size);

	Rect2(int x, int y, int width, int height);
};
#pragma endregion
class Colour {
public:
	unsigned char r, g, b, a;
	
	Colour();
	
	explicit Colour(unsigned char v);
	
	explicit Colour(std::string hex);
	
	Colour(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	
	static Colour fromFloat(float r, float g, float b, float a = 1.);
	
	std::string getHex(bool with_alpha = false);
	
	static const Colour NONE;
	static const Colour WHITE;
	static const Colour BLACK;
};

#endif//CELESTEPET_DATA_TYPE_H
