//
// Created by School on 2025/3/14.
//

#include "vulkan/vk_engine.h"

Game::GraphicsEngine* loadedEngine = nullptr;
constexpr bool bUseValidationLayers = true;

void Game::GraphicsEngine::init(Game::Window& pWindow) {
	std::printf("Started Vulkan initialisation\n");
	
	// only one engine initialization is allowed with the application.
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	mWindow = &pWindow;
	
	initVulkan(pWindow);
	
	const Rect2<int> screenRect = pWindow.getScreenRect();
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
		//make sure the gpu has stopped doing its things
		vkDeviceWaitIdle(device);
		
		for (auto & frame : frames) {
			//already written from before
			vkDestroyCommandPool(device, frame.commandPool, nullptr);
			
			//destroy sync objects
			vkDestroyFence(device, frame.renderFence, nullptr);
			vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
			vkDestroySemaphore(device ,frame.swapchainSemaphore, nullptr);
		}
		destroySwapchain();
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
		
		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);
		if (mWindow) {
			delete(mWindow);
			mWindow = nullptr;
		}
	}
	
	// clear engine pointer
	loadedEngine = nullptr;
}

void Game::GraphicsEngine::draw() {
	#pragma region Draw 1
		// Wait until the gpu has finished rendering the last frame. Timeout of 1
		// second
		VK_CHECK(vkWaitForFences(device, 1, &getCurrentFrame().renderFence, VK_TRUE, 1'000'000'000u));
		VK_CHECK(vkResetFences(device, 1, &getCurrentFrame().renderFence));
	#pragma endregion
	
	#pragma region Draw 2
		//request image from the swapchain
		uint32_t swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(
            device, swapchain, 1'000'000'000, getCurrentFrame().swapchainSemaphore,
			nullptr, &swapchainImageIndex
		));
	#pragma endregion

	#pragma region Draw 3
		//naming it cmd for shorter writing
		VkCommandBuffer cmd = getCurrentFrame().mainCommandBuffer;
		
		// now that we are sure that the commands finished executing, we can safely
		// reset the command buffer to begin recording again.
		VK_CHECK(vkResetCommandBuffer(cmd, 0));
		
		//Begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
		VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		
		//start the command buffer recording
		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
	#pragma endregion

	#pragma region Draw 4
		//make the swapchain image into writeable mode before rendering
	    VkUtil::transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		
		//Make a clear-color from frame number. This will flash with a 120 frame period.
		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(static_cast<float>(frameNumber) / 120.f));
		clearValue = { { 0.0f, 0.0f, flash, 0.0f } };
		
		VkImageSubresourceRange clearRange = VkInit::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
		
		//clear image
		vkCmdClearColorImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
		
		//make the swapchain image into presentable mode
	    VkUtil::transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		
		//finalize the command buffer (we can no longer add commands, but it can now be executed)
		VK_CHECK(vkEndCommandBuffer(cmd));
	#pragma endregion

	#pragma region Draw 5
		//Prepare the submission to the queue.
		//We want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		//we will signal the _renderSemaphore, to signal that rendering has finished
		
		VkCommandBufferSubmitInfo cmdinfo = VkInit::commandBufferSubmitInfo(cmd);
		
		VkSemaphoreSubmitInfo waitInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().renderSemaphore);
		
		VkSubmitInfo2 submit = VkInit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);
		
		//Submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submit, getCurrentFrame().renderFence));
	#pragma endregion
	
	#pragma region Draw 6
		//Prepare present
		// this will put the image we just rendered to into the visible window.
		// We want to wait on the _renderSemaphore for that,
		// as its necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.swapchainCount = 1;
		
		presentInfo.pWaitSemaphores = &getCurrentFrame().renderSemaphore;
		presentInfo.waitSemaphoreCount = 1;
		
		presentInfo.pImageIndices = &swapchainImageIndex;
		
		VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));
		
		//increase the number of frames drawn
		frameNumber++;
	#pragma endregion
}

void Game::GraphicsEngine::initVulkan(const Game::Window& window) {
	vkb::InstanceBuilder instanceBuilder;
	auto instanceRet = instanceBuilder
	                            .set_app_name("Celeste Pet")
	                            .request_validation_layers(bUseValidationLayers)
	                            .require_api_version(1,3,0)
	                            .build(); // build is always called last

	// simple error checking and helpful error messages
	if (!instanceRet) {
		std::cerr << "Failed to create Vulkan instance. Error: " << instanceRet.error().message() << "\n";
		throw std::exception();
	}
	
	const vkb::Instance vkbInst = instanceRet.value();

	instance = vkbInst.instance;
	debugMessenger = vkbInst.debug_messenger;
	
	window.getVulkanSurface(instance, &surface);
	
	//vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;
	
	//vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;
	
	//Use vkbootstrap to select a gpu.
	//We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{vkbInst};
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
	chosenGpu = physicalDevice.physical_device;
	
	// use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	std::printf("supported composite alpha: %u\n", surfaceCapabilities.supportedCompositeAlpha);
}

void Game::GraphicsEngine::createSwapchain(uint32_t width, uint32_t height) {
	vkb::SwapchainBuilder swapchainBuilder{chosenGpu, device, surface };
	
	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.set_composite_alpha_flags(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) //TODO???
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
	//Create a command pool for commands submitted to the graphics queue.
	//We also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = VkInit::commandPoolCreateInfo(
	        graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	for (auto& frame : frames) {
		
		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frame.commandPool));
		
		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(frame.commandPool, 1);
		
		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frame.mainCommandBuffer));
	}
}
void Game::GraphicsEngine::initSyncStructures() {
	//create synchronisation structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to synchronise rendering with swapchain
	//we want the fence to start signalled, so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = VkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = VkInit::semaphoreCreateInfo();
	
	for (auto& frame : frames) {
		VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.renderFence));
		
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSemaphore));
	}
}