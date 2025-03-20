//
// Created by School on 2025/3/14.
//

#ifndef MADLINEENGINE_VK_ENGINE_H
#define MADLINEENGINE_VK_ENGINE_H

#include <ranges>

#include "vulkan/vk_types.h"
#include "vk_descriptors.h"

#include "glfw/game_window.h"
#include "glfw/data_type.h"

constexpr unsigned int FRAME_OVERLAP = 2;

namespace Madline {
	struct DeletionQueue {
		// To improve, we could instead store the handles (pointers) to the objects, then delete them
		std::deque<std::function<void()>> deletors;
		
		void pushFunction(std::function<void()>&& function) {
			deletors.push_back(function);
		}
		
		void flush() {
			// reverse iterate the deletion queue to execute all the functions
			for (auto& deletor : std::ranges::reverse_view(deletors)) {
				deletor(); //call functors
			}
	
			deletors.clear();
		}
	};
	
	struct FrameData {
		VkCommandPool commandPool;
		VkCommandBuffer mainCommandBuffer;
		VkSemaphore swapchainSemaphore, renderSemaphore;
		VkFence renderFence;
		DeletionQueue deletionQueue;
	};
	
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
	
		std::array<FrameData, 2> frames;
		FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };
		
		VkQueue graphicsQueue{};
		uint32_t graphicsQueueFamily{};
		
		VkSurfaceKHR surface{};// Vulkan mWindow surface
		VkSwapchainKHR swapchain{};
		VkFormat swapchainImageFormat{};
		VkExtent2D swapchainExtent{};
		VkExtent2D drawExtent{};
	
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		
		VkDescriptorSet drawImageDescriptors{};
		VkDescriptorSetLayout drawImageDescriptorLayout{};
		
		VkPipeline gradientPipeline{};
		VkPipelineLayout gradientPipelineLayout{};
		
		// immediate submit structures
		VkFence immFence{};
		VkCommandBuffer immCommandBuffer{};
		VkCommandPool immCommandPool{};
		
		DeletionQueue mainDeletionQueue;
		
		VmaAllocator allocator{};
		DescriptorAllocator globalDescriptorAllocator{};
		
		AllocatedImage drawImage{};
		
		void initVulkan(const Madline::Window& window);
		void initSwapchain();
		void initCommands();
		void initSyncStructures();
		void initDescriptors();
		void initPipelines();
		void initBackgroundPipelines();
		void initImgui();
		
		static std::vector<const char*> getRequiredExtensions();
		
		void createSwapchain(uint32_t width, uint32_t height);
		void destroySwapchain();
		
		void draw();
		void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView);
		void drawBackground(VkCommandBuffer cmd) const;
		
		void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	public:
		//initializes everything in the engine
		void init(Madline::Window&pWindow);
		
		//shuts down the engine
		void cleanup();

		//draw loop
		void drawLoop();
		
		GraphicsEngine() = default;
		
		~GraphicsEngine();
	};
}

namespace GEngineTools {
	void exitOnError(std::string msg);
}

#endif//MADLINEENGINE_VK_ENGINE_H
