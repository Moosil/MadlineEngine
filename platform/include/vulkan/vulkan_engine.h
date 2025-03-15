//
// Created by School on 2025/3/14.
//

#ifndef RENDERER_VULKAN
#define RENDERER_VULKAN

#include <vkbootstrap/VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <iostream>

namespace GraphicsEngine {
	class Vulkan {
	private:
		VkInstance instance;
		
		void initInstance();
	public:
		Vulkan();
		virtual ~Vulkan();
	};
}

namespace GEngineTools {
	void exitOnError(std::string msg);
}

#endif//RENDERER_VULKAN
