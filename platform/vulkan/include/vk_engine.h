//
// Created by School on 2025/3/14.
//

#ifndef CELESTEPET_VK_ENGINE_H
#define CELESTEPET_VK_ENGINE_H

#include <VkBootstrap/VkBootstrap.h>

class GraphicsEngine {
public:
	VkInstance _instance; // Vulkan library handle
	VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice _chosenGPU;// GPU chosen as the default device
	VkDevice _device; // Vulkan device for commands
	VkSurfaceKHR _surface;// Vulkan window surface
private:
};

#endif//CELESTEPET_VK_ENGINE_H
