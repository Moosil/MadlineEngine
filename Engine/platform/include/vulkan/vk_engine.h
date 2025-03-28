//
// Created by School on 2025/3/14.
//

#ifndef MADLINEENGINE_VK_ENGINE_H
#define MADLINEENGINE_VK_ENGINE_H

#include <ranges>

#include "vulkan/vk_types.h"
#include "vk_descriptors.h"

#include "winapi/game_window.h"
#include "winapi/data_type.h"

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
	
	struct ComputePushConstants {
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};
	
	struct ComputeEffect {
		std::string name;
		
		VkPipeline pipeline;
		VkPipelineLayout layout;

		ComputePushConstants data;
	};
	
	struct PipelineShader {
	
	};
	
	struct SurfaceComponents {
		HWND handle{};
		int frameNumber {0};
		bool hasImgui { false };
		bool drawingEnabled { false };
		
		VkSurfaceKHR surface{};
		VkSwapchainKHR swapchain{};
		VkFormat swapchainImageFormat{};
		VkExtent2D swapchainExtent{};
		VkExtent2D drawExtent{};
		
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		
		std::array<FrameData, 2> frames;
		FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };
		
		VkPipeline gradientPipeline{};
		VkPipelineLayout gradientPipelineLayout{};
		
		VkQueue graphicsQueue{};
		uint32_t graphicsQueueFamily{};
		
		VkDescriptorSet drawImageDescriptors{};
		VkDescriptorSetLayout drawImageDescriptorLayout{};
		
		// immediate submit structures
		VkFence immFence{};
		VkCommandBuffer immCommandBuffer{};
		VkCommandPool immCommandPool{};
		
		DeletionQueue mainDeletionQueue;
		
		VmaAllocator allocator{};
		DescriptorAllocator globalDescriptorAllocator{};
		
		AllocatedImage drawImage{};
		
		std::vector<ComputeEffect> backgroundEffects;
		int currentBackgroundEffect{ 0 };
		
		void (*createImguiStuff)(SurfaceComponents&) = nullptr;
	};

	
	class GraphicsEngine {
	private:
		bool isInitialized { false };
		
		Window* mWindow{ nullptr };
		VkExtent2D windowExtent{ sizeof(VkExtent2D) };
		
		VkInstance instance{};// Vulkan library handle
		VkDebugUtilsMessengerEXT debugMessenger{};// Vulkan debug output handle
		VkPhysicalDevice chosenGpu{};// GPU chosen as the default device
		VkDevice device{}; // Vulkan device for commands
		
		std::unordered_map<std::string, SurfaceComponents> surfaces{};
		
		void initVulkan(const Madline::Window& window);
		void initSwapchain(SurfaceComponents &surfComp);
		void initCommands(SurfaceComponents &surfComp);
		void initSyncStructures(SurfaceComponents &surfComp);
		void initDescriptors(SurfaceComponents &surfComp);
		void initPipelines(SurfaceComponents &surfComp);
		void initBackgroundPipelines(SurfaceComponents &surfComp);
		void initImgui(SurfaceComponents &surfComp);
		
		void createSwapchain(uint32_t width, uint32_t height, SurfaceComponents &surfComp);
		void destroySwapchain(SurfaceComponents &surfComp);
		
		void draw(SurfaceComponents &surfComp);
		void drawImgui(VkCommandBuffer cmd, VkImageView targetImageView, SurfaceComponents &surfComp);
		void drawBackground(VkCommandBuffer cmd, SurfaceComponents &surfComp) const;
		
		void immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function,
		                     SurfaceComponents &surfComp);
		
		void createComputeShader(const std::string &shaderName, const ComputePushConstants &pushConstants,
		                         VkPipelineShaderStageCreateInfo *stageInfo,
		                         VkComputePipelineCreateInfo *computePipelineCreateInfo,
		                         SurfaceComponents &surfComp);
		
		static void mainWindowCreateImgui(SurfaceComponents &surfComp);
	public:
		//initializes everything in the engine
		void init(Madline::Window& pWindow);
		
		//shuts down the engine
		void cleanup();

		//draw loop
		void drawLoop();
		
		explicit GraphicsEngine(Madline::Window& pWindow);
		
		~GraphicsEngine();
	};
}

namespace GEngineTools {
	void exitOnError(std::string msg);
}

#endif//MADLINEENGINE_VK_ENGINE_H
