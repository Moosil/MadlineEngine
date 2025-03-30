//
// Created by School on 2025/3/11.
//

#ifndef CELESTEPET_GAME_LOGIC_H
#define CELESTEPET_GAME_LOGIC_H

#include <format>

#include "MadlineEngine.h"

namespace Madline {
	class Engine {
	public:
		virtual ~Engine() = default;
		
		virtual bool initGameplay() = 0;

		virtual bool process(float delta, Window& gameWindow) = 0;

		virtual void onCloseGame() = 0;
	};
	
	class EngineDebug : public Engine {
	public:
		bool initGameplay() override;
		
		bool process(float delta, Window& gameWindow) override;

		void onCloseGame() override;
	};
}

#endif//CELESTEPET_GAME_LOGIC_H
