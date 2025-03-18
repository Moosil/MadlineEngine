//
// Created by School on 2025/3/18.
//

#ifndef CELESTEPET_GAME_ENGINE_H
#define CELESTEPET_GAME_ENGINE_H

#include "CelestePetHeaders.h"

namespace Game {
	int startGame(Game::Engine &gameEngine);

	std::string getLastErrorString();

	void errorPrint(const std::string &message);

	size_t getFileSize(const std::string &name);

	bool readEntireFile(const std::string &name, std::vector<char> &data);

	bool writeEntireFile(const std::string &name, void *buffer, size_t s);
}

#endif//CELESTEPET_GAME_ENGINE_H
