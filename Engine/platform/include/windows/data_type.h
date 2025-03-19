//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_DATA_TYPE_H
#define CELESTEPET_DATA_TYPE_H


#include <glm.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif//_WIN32

namespace Madline {
	glm::vec<2, int> vec2iFromPoint(POINT point);

#pragma region Rect
	template<typename T>
	class Rect2 {
	public:
		glm::vec<2, T> pos, size;

		Rect2() : pos(), size(){};

		Rect2(const Rect2 &other) = default;
		
#ifdef _WIN32
		template<std::enable_if_t<std::is_integral_v<T>, bool> = true>
		explicit Rect2<T>(const RECT& rect):
            pos(glm::vec<2, T>(rect.left, rect.top)),
            size(glm::vec<2, T>(rect.right - rect.left, rect.bottom - rect.top)) {}
#endif
		
		template<typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
		explicit Rect2(const Rect2<U> &other):
            pos(glm::vec<2, T>(static_cast<T>(other.pos.x), static_cast<T>(other.pos.y))),
			size(glm::vec<2, T>(static_cast<T>(other.size.x), static_cast<T>(other.size.y))) {}

		Rect2(glm::vec<2, T> pos, glm::vec<2, T> size) : pos(pos), size(size){};

		Rect2(T x, T y, T width, T height) : pos(Vector2<T>(x, y)), size(Vector2<T>(width, height)){};

		bool operator==(const Rect2 &other) const {
			return pos == other.pos && size == other.size;
		}

		bool operator!=(const Rect2 &other) const {
			return pos != other.pos || size != other.size;
		}
		
		[[nodiscard]] T getWidth() const {
			return size.x;
		}
		
		[[nodiscard]] T getHeight() const {
			return size.y;
		}
	};
#pragma endregion
	class Colour {
	public:
		unsigned char r, g, b, a;

		Colour();
		
		template<typename T, typename U,
	         std::enable_if_t<std::is_convertible_v<T, unsigned char>, bool> = true,
	         std::enable_if_t<std::is_convertible_v<U, unsigned char>, bool> = true
		>
		explicit Colour(const T& v, const U& a = 255);
		
		template<typename T, std::enable_if_t<std::is_convertible_v<T, unsigned char>, bool> = true>
		explicit Colour(const T& r, const T& g, const T& b, const T& a = 255);

		explicit Colour(std::string hex);

		std::string getHex(bool withAlpha = false);

		static const Colour NONE;
		static const Colour WHITE;
		static const Colour BLACK;
	};
}

#endif//CELESTEPET_DATA_TYPE_H
