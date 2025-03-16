//
// Created by School on 2025/3/14.
//

#include "vulkan/vk_engine.h"

Game::GraphicsEngine* loadedEngine = nullptr;
constexpr bool bUseValidationLayers = true;

void Game::GraphicsEngine::init(Game::Window& p_window) {
	std::printf("Started Vulkan initialisation\n");
	
	// only one engine initialization is allowed with the application.
	assert(loadedEngine == nullptr);
	loadedEngine = this;
	
	m_window = &p_window;
	
	initVulkan(p_window);
	
	const Rect2<int> screenRect = p_window.getScreenRect();
	windowExtent.height = screenRect.getHeight();
	windowExtent.width = screenRect.getWidth();
	
	initSwapchain();
	
	initCommands();
	
	initSyncStructures();
	
	isInitialized = true;
	std::printf("Finished Vulkan initialisation\n");
}

void Game::GraphicsEngine::cleanup() {
	if (isInitialized) {
		destroySwapchain();
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
		
		vkb::destroy_debug_utils_messenger(instance, debug_messenger);
		vkDestroyInstance(instance, nullptr);
		if (m_window) {
			delete(m_window);
			m_window = nullptr;
		}
	}
	
	// clear engine pointer
	loadedEngine = nullptr;
}

void Game::GraphicsEngine::draw() {
	// nothing yet
}


void Game::GraphicsEngine::initVulkan(const Game::Window& window) {
	vkb::InstanceBuilder instance_builder;
	auto instance_ret = instance_builder
	                            .set_app_name("Celeste Pet")
	                            .request_validation_layers(bUseValidationLayers)
	                            .require_api_version(1,3,0)
	                            .build(); // build is always called last

	// simple error checking and helpful error messages
	if (!instance_ret) {
		std::cerr << "Failed to create Vulkan instance. Error: " << instance_ret.error().message() << "\n";
		throw std::exception();
	}
	
	const vkb::Instance vkb_inst = instance_ret.value();

	instance = vkb_inst.instance;
	debug_messenger = vkb_inst.debug_messenger;
	
	window.getVulkanSurface(instance, &surface);
	
	//vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;
	
	//vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;
	
	//use vkbootstrap to select a gpu.
	//We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
	                                             .set_minimum_version(1, 3)
	                                             .set_required_features_13(features)
	                                             .set_required_features_12(features12)
	                                             .set_surface(surface)
	                                             .select()
	                                             .value();
	

	//create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	device = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;
}

void Game::GraphicsEngine::createSwapchain(uint32_t width, uint32_t height) {
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };
	
	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	
	vkb::Swapchain vkbSwapchain = swapchainBuilder
	                                      //.use_default_format_selection()
	                                      .set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
	                                      //use vsync present mode
	                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	                                      .set_desired_extent(width, height)
	                                      .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	                                      .build()
	                                      .value();
	
	swapchainExtent = vkbSwapchain.extent;
	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Game::GraphicsEngine::destroySwapchain() {
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	
	// destroy swapchain resources
	for (auto& swapchainImageView : swapchainImageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}
}

void Game::GraphicsEngine::initSwapchain() {
	createSwapchain(windowExtent.width, windowExtent.height);
}

void Game::GraphicsEngine::initCommands() {
	//nothing yet
}
void Game::GraphicsEngine::initSyncStructures() {
	//nothing yet
}