//
// Created by School on 2025/3/14.
//

#ifndef CELESTEPET_CELESTEPETHEADERS_H
#define CELESTEPET_CELESTEPETHEADERS_H

#include <ostream>
#include <format>
#include <algorithm>
#include <array>
#include <cassert>
#include <exception>
#include <iostream>
#include <vector>

#include <glm.hpp>

#ifdef _WIN32
#include <Windows.h>

#include "windows/data_type.h"
#include "windows/input.h"
#include "windows/game_window.h"
#endif// _WIN32

#ifdef RENDER_VULKAN
#include "vulkan/vk_engine.h"
#endif

#include "game_logic.h"
#include "madline.h"

#include "CelestePetConsts.h"

#endif//CELESTEPET_CELESTEPETHEADERS_H
