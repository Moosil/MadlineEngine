//
// Created by School on 2025/3/11.
//

#ifndef MADLINEENGINE_DATA_TYPE_H
#define MADLINEENGINE_DATA_TYPE_H


#include <glm.hpp>

namespace Madline {
#pragma region Rect

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCInconsistentNamingInspection"
	template<typename T = float, glm::qualifier Q = glm::defaultp>
	using Vec2 = glm::tvec2<T, Q>;
	using Vec2f = glm::tvec2<float, glm::defaultp>;
	using Vec2i = glm::tvec2<int, glm::defaultp>;
#pragma clang diagnostic pop
	
	template<typename T>
	class Rect2 {
	public:
		Vec2<T> pos, size;

		Rect2() : pos(), size(){};

		Rect2(const Rect2 &other) = default;
		
		template<typename U, std::enable_if_t<std::is_convertible_v<U, T>, bool> = true>
		explicit Rect2(const Rect2<U> &other):
            pos(Vec2<T>(static_cast<T>(other.pos.x), static_cast<T>(other.pos.y))),
			size(Vec2<T>(static_cast<T>(other.size.x), static_cast<T>(other.size.y))) {}

		Rect2(Vec2<T> pos, Vec2<T> size) : pos(pos), size(size){};

		Rect2(T x, T y, T width, T height) : pos(Vec2<T>(x, y)), size(Vec2<T>(width, height)){};
		
		Rect2(T width, T height) : pos(Vec2<T>(0, 0)), size(Vec2<T>(width, height)) {};

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

		explicit operator glm::vec4() const;
		
		std::string getHex(bool withAlpha = false);
		
		static const Colour NONE;
		static const Colour WHITE;
		static const Colour BLACK;
	};
}

#endif//MADLINEENGINE_DATA_TYPE_H
