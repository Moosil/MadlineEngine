//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_DATA_TYPE_H
#define CELESTEPET_DATA_TYPE_H

#include <ostream>
#include <windows.h>

#pragma region Vector
template<typename T>
class Vector2 {
public:
	T x, y;

	Vector2() : x(0), y(0){};

	explicit Vector2(T val) : x(val), y(val){};

	Vector2(T x, T y) : x(x), y(y){};

	Vector2(const Vector2 &from) : x(from.x), y(from.y){};

	bool operator==(const Vector2 &other) const {
		return x == other.x && y == other.y;
	}

	bool operator!=(const Vector2 &other) const {
		return x != other.x || y != other.y;
	}

	bool operator>(const Vector2 &other) const {
		if (x == other.x) {
			return y > other.y;
		} else {
			return x > other.x;
		}
	}

	bool operator<(const Vector2 &other) const {
		if (x == other.x) {
			return y < other.y;
		} else {
			return x < other.x;
		}
	}

	bool operator>=(const Vector2 &other) const {
		if (x == other.x) {
			return y >= other.y;
		} else {
			return x >= other.x;
		}
	}

	bool operator<=(const Vector2 &other) const {
		if (x == other.x) {
			return y <= other.y;
		} else {
			return x <= other.x;
		}
	}

	Vector2 operator[](int idx) {
		if (idx == 0) {
			return x;
		} else if (idx == 1) {
			return y;
		} else {
			return nullptr;
		}
	}

	Vector2 operator+() const {
		return this;
	}

	Vector2 operator+(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-() const {
		return this * -1;
	}

	Vector2 operator-(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator*(T scalar) const {
		return Vector2(x * scalar, y * scalar);
	}

	Vector2 operator*(const Vector2 &other) const {
		return Vector2(x * other.x, y * other.y);
	}

	Vector2 operator/(T scalar) const {
		return Vector2(x / scalar, y / scalar);
	}

	Vector2 operator/(const Vector2 &other) const {
		return Vector2(x / other.x, y / other.y);
	}

	friend std::ostream &operator<<(std::ostream &out, const Vector2<T> vector2) {
		out << '(' << vector2.x << ',' << vector2.y << ')';
		return out;
	}

	inline static const Vector2 ZERO{};
	inline static const Vector2 ONE{1};
	inline static const Vector2 UP{0, -1};
	inline static const Vector2 DOWN{0, 1};
	inline static const Vector2 LEFT{-1, 0};
	inline static const Vector2 RIGHT{1, 0};
};

template<>
class Vector2<float> {
public:
	float x, y;

	Vector2();

	explicit Vector2(float val);

	explicit Vector2(const Vector2<int> &other);

	Vector2(float x, float y);

	Vector2(const Vector2 &other) = default;

	Vector2<float> operator*(int scalar) const;

	Vector2<float> operator/(int scalar) const;

	friend std::ostream &operator<<(std::ostream &out, const Vector2<float> vector2);

	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UP;
	static const Vector2 DOWN;
	static const Vector2 LEFT;
	static const Vector2 RIGHT;
};

template<>
class Vector2<int> {
public:
	int x, y;

	Vector2();

	explicit Vector2(int val);

	Vector2(POINT point);

	explicit Vector2(const Vector2<float> &other);

	Vector2(int x, int y);

	Vector2(const Vector2 &other) = default;

	Vector2<float> operator*(float scalar) const;

	Vector2<float> operator*(const Vector2<float> &other) const;

	Vector2<float> operator/(float scalar) const;

	Vector2<float> operator/(const Vector2<float> &other) const;

	friend std::ostream &operator<<(std::ostream &out, const Vector2<int> vector2);

	static const Vector2 ZERO;
	static const Vector2 ONE;
	static const Vector2 UP;
	static const Vector2 DOWN;
	static const Vector2 LEFT;
	static const Vector2 RIGHT;
};
#pragma endregion
#pragma region Rect
template<typename T>
class Rect2 {
public:
	Vector2<T> pos, size;

	Rect2() : pos(), size(){};

	Rect2(const Rect2 &other) = default;

	Rect2(Vector2<T> pos, Vector2<T> size) : pos(pos), size(size){};

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
	Vector2<float> pos, size;
	
	Rect2();
	
	Rect2(const Rect2 &other) = default;
	
	explicit Rect2(const Rect2<int> &other);

	Rect2(Vector2<float> pos, Vector2<float> size);

	Rect2(float x, float y, float width, float height);
};

template<>
class Rect2<int> {
public:
	Vector2<int> pos, size;

	Rect2();
	
	Rect2(RECT rect);
	
	Rect2(const Rect2 &other) = default;
	
	explicit Rect2(const Rect2<float> &other);
	
	Rect2(Vector2<int> pos, Vector2<int> size);

	Rect2(int x, int y, int width, int height);
};
#pragma endregion
class Colour {
public:
	unsigned char r, g, b, a;
	
	Colour();
	
	explicit Colour(unsigned char v);
	
	Colour(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	
	static Colour fromFloat(float r, float g, float b, float a = 1.);
	
	explicit Colour(std::string hex);
	
	std::string getHex(bool with_alpha = false);
	
	static const Colour NONE;
	static const Colour WHITE;
	static const Colour BLACK;
};

#endif//CELESTEPET_DATA_TYPE_H
