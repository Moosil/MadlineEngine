//
// Created by School on 2025/3/14.
//

#ifndef RENDERER_VULKAN
#define RENDERER_VULKAN

#include <chrono>
#include <thread>
#include <iostream>

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include "vk_initialisers.h"
#include "vk_types.h"
#include "vulkan/vk_images.h"

#ifdef _WIN32
#include "windows/game_window.h"
#endif//_WIN32

struct FrameData {
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

namespace Game {
	class GraphicsEngine {
	private:
		bool isInitialized { false };
		int frameNumber {0};
		VkExtent2D windowExtent{ sizeof(VkExtent2D) };
		Window* mWindow{ nullptr };
		
		VkInstance instance{};// Vulkan library handle
		VkDebugUtilsMessengerEXT debugMessenger{};// Vulkan debug output handle
		VkPhysicalDevice chosenGpu{};// GPU chosen as the default device
		VkDevice device{}; // Vulkan device for commands
		VkSurfaceKHR surface{};// Vulkan window surface
		
		VkSwapchainKHR swapchain;
		VkFormat swapchainImageFormat;
	
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		VkExtent2D swapchainExtent;
	
		FrameData frames[FRAME_OVERLAP];
	
		FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };
	
		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamily;
		
		void initVulkan(const Game::Window& window);
		void initSwapchain();
		void initCommands();
		void initSyncStructures();
		
		void createSwapchain(uint32_t width, uint32_t height);
		void destroySwapchain();
	public:
		//initializes everything in the engine
		void init(Game::Window&pWindow);
		
		//shuts down the engine
		void cleanup();

		//draw loop
		void draw();
	};
}

namespace GEngineTools {
	void exitOnError(std::string msg);
}

#endif//RENDERER_VULKAN
