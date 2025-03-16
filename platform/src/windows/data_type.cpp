//
// Created by School on 2025/3/11.
//

#include "windows/data_type.h"

glm::vec<2, int> Game::vec2iFromPoint(POINT point) {
	return {point.x, point.y};
}

#pragma region Colour
Game::Colour::Colour(): r(0), g(0), b(0), a(0) {}

template<typename T, typename U,
	std::enable_if_t<std::is_convertible_v<T, unsigned char>, bool>,
	std::enable_if_t<std::is_convertible_v<U, unsigned char>, bool>
>
Game::Colour::Colour(const T &v, const U &a):
	r(static_cast<unsigned char>(v)),
	g(static_cast<unsigned char>(v)),
	b(static_cast<unsigned char>(v)),
	a(static_cast<unsigned char>(a)){}

template<typename T, std::enable_if_t<std::is_convertible_v<T, unsigned char>, bool>>
Game::Colour::Colour(const T &r, const T &g, const T &b, const T &a):
	r(static_cast<unsigned char>(r)),
	g(static_cast<unsigned char>(g)),
	b(static_cast<unsigned char>(b)),
	a(static_cast<unsigned char>(a)){}

Game::Colour::Colour(std::string hex) {
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

std::string Game::Colour::getHex(bool with_alpha) {
	std::string hex = std::format("{:x}", r) + std::format("{:x}", g) + std::format("{:x}", b);
	if (with_alpha) {
		hex += std::format("{:x}", a);
	}
	return hex;
}
#pragma region Constants
const Game::Colour Game::Colour::NONE{0,0,0,0};
const Game::Colour Game::Colour::WHITE{255,255,255,255};
const Game::Colour Game::Colour::BLACK{0,0,0,255};
#pragma endregion
#pragma endregion
