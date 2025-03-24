//
// Created by School on 2025/3/14.
//

#include <chrono>
#include <ranges>
#include <thread>
#include <iostream>
#include <format>

#include "CelestePetConsts.h"

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include "winapi/game_window.h"

#include "vulkan/vk_images.h"
#include "vulkan/vk_initialisers.h"
#include "vulkan/vk_pipelines.h"

#include "vulkan/vk_engine.h"


Madline::GraphicsEngine* loadedEngine = nullptr;
#if BUILD_TYPE == Debug
constexpr bool USE_VALIDATION_LAYERS = true;
#else
constexpr bool USE_VALIDATION_LAYERS = false;
#endif

Madline::GraphicsEngine::GraphicsEngine(Madline::Window& pWindow) {
	init(pWindow);
}

Madline::GraphicsEngine::~GraphicsEngine() {
	cleanup();
}

void Madline::GraphicsEngine::init(Madline::Window& pWindow) {
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
	
	initDescriptors();
	
	initPipelines();
	
	initImgui();
	
	isInitialized = true;
	std::printf("Finished Vulkan initialisation\n");
}

void Madline::GraphicsEngine::cleanup() {
	if (isInitialized) {
		
		//make sure the gpu has stopped doing its things
		vkDeviceWaitIdle(device);
		
		//free per-frame structures and deletion queue
		for (auto & frame : frames) {
			
			vkDestroyCommandPool(device, frame.commandPool, nullptr);

			//destroy sync objects
			vkDestroyFence(device, frame.renderFence, nullptr);
			vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
			vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);

			frame.deletionQueue.flush();
		}

		//flush the global deletion queue
		mainDeletionQueue.flush();
		
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

void Madline::GraphicsEngine::drawLoop() {
	// imgui new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	if (ImGui::Begin("background")) {
		
		ComputeEffect& selected = backgroundEffects[currentBackgroundEffect];
		
		ImGui::Text("Selected effect: ", selected.name.c_str());
		
		ImGui::SliderInt("Effect Index", &currentBackgroundEffect,0, static_cast<int>(backgroundEffects.size() - 1));
		
		ImGui::InputFloat4("data1",(float*)& selected.data.data1);
		ImGui::InputFloat4("data2",(float*)& selected.data.data2);
		ImGui::InputFloat4("data3",(float*)& selected.data.data3);
		ImGui::InputFloat4("data4",(float*)& selected.data.data4);
	}
	ImGui::End();
	
	ImGui::Render();
	
	//our draw function
	draw();
}

void Madline::GraphicsEngine::drawBackground(VkCommandBuffer cmd) const {
	ComputeEffect effect = backgroundEffects[currentBackgroundEffect];
	
	// bind the background compute pipeline
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

	// bind the descriptor set containing the draw image for the compute pipeline
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradientPipelineLayout, 0, 1,
		&drawImageDescriptors, 0, nullptr
    );

	vkCmdPushConstants(cmd, gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
	// Execute the compute pipeline dispatch. We are using 16x16 workgroup size, so we need to divide by it
	vkCmdDispatch(cmd, std::ceil(static_cast<float>(drawExtent.width) / 16.0),
		std::ceil(static_cast<float>(drawExtent.height) / 16.0), 1
	);


}

void Madline::GraphicsEngine::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView) {
	VkRenderingAttachmentInfo colorAttachment = VkInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = VkInit::renderingInfo(swapchainExtent, &colorAttachment, nullptr);
	
	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}


void Madline::GraphicsEngine::draw() {
	#pragma region Flush deletion queue
		//Wait until the gpu has finished rendering the last frame. Timeout of 1 second
		VK_CHECK(vkWaitForFences(device, 1, &getCurrentFrame().renderFence, true, 1000000000));
		
		getCurrentFrame().deletionQueue.flush();
	#pragma endregion
	
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

	#pragma region Record command buffer
	    drawExtent.width = drawImage.imageExtent.width;
	    drawExtent.height = drawImage.imageExtent.height;
	    
	    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	    // transition our main draw image into general layout, so we can write into it,
	    // we will overwrite it all, so we don't care about what was the older layout
	    VkUtil::transitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	    drawBackground(cmd);

	    //transition the draw image and the swapchain image into their correct transfer layouts
	    VkUtil::transitionImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	    VkUtil::transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	    // execute a copy from the draw image into the swapchain
	    VkUtil::copyImageToImage(cmd, drawImage.image, swapchainImages[swapchainImageIndex], drawExtent, swapchainExtent);
	    
	    // set swapchain image layout to Attachment Optimal, so we can draw it
	    VkUtil::transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	#pragma endregion

	#pragma region Draw Imgui
	    
	    //draw imgui into the swapchain image
	    drawImgui(cmd,  swapchainImageViews[swapchainImageIndex]);

	    // set swapchain image layout to Present, so we can draw it
	    VkUtil::transitionImage(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	    //finalize the command buffer (we can no longer add commands, but it can now be executed)
	    VK_CHECK(vkEndCommandBuffer(cmd));
		#pragma endregion

	#pragma region Draw 5
		//Prepare the submission to the queue.
		//We want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		//we will signal the _renderSemaphore, to signal that rendering has finished
		
		VkCommandBufferSubmitInfo cmdInfo = VkInit::commandBufferSubmitInfo(cmd);
		
		VkSemaphoreSubmitInfo waitInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().renderSemaphore);
		
		VkSubmitInfo2 submit = VkInit::submitInfo(&cmdInfo, &signalInfo, &waitInfo);
		
		//Submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submit, getCurrentFrame().renderFence));
	#pragma endregion
	
	#pragma region Draw 6
		//Prepare present
		// this will put the image we just rendered to into the visible mWindow.
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

void Madline::GraphicsEngine::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VK_CHECK(vkResetFences(device, 1, &immFence));
	VK_CHECK(vkResetCommandBuffer(immCommandBuffer, 0));
	
	VkCommandBuffer cmd = immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = VkInit::commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = VkInit::submitInfo(&cmdinfo, nullptr, nullptr);

	// Submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submit, immFence));

	VK_CHECK(vkWaitForFences(device, 1, &immFence, true, 9999999999));
}


void Madline::GraphicsEngine::initVulkan(const Madline::Window& window) {
	
	// Create VkBootstrap InstanceBuilder
	vkb::InstanceBuilder instanceBuilder;
	auto instanceRet = instanceBuilder
		.set_app_name(GAME_NAME)
		.set_app_version(1, 0, 0)
		.set_engine_name("MadlineEngine")
		.set_engine_version(1, 0, 0)
		.require_api_version(1,3,0)
		.request_validation_layers(USE_VALIDATION_LAYERS)
		.build(); // build is always called last

	// Check if instance is null
	if (!instanceRet) {
		throw std::runtime_error(std::format("Failed to create Vulkan instance. Error: {}", instanceRet.error().message()));
	}
	
	// Get the built instance
	const vkb::Instance vkbInstance = instanceRet.value();

	instance = vkbInstance.instance;
	debugMessenger = vkbInstance.debug_messenger;
	
	// Create Windows api surface for Vulkan. If surface is non created successfully, throw a runtime error
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
	vkb::PhysicalDeviceSelector selector{ vkbInstance };
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
	
	#pragma region VMA init
		// initialize the memory allocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = chosenGpu;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &allocator);
		
		mainDeletionQueue.pushFunction([&]() {
			vmaDestroyAllocator(allocator);
		});
	#pragma endregion
	
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	std::printf("supported composite alpha: %u\n", surfaceCapabilities.supportedCompositeAlpha);
}

void Madline::GraphicsEngine::destroySwapchain() {
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	
	// destroy swapchain resources
	for (auto& swapchainImageView : swapchainImageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}
}

void Madline::GraphicsEngine::createSwapchain(uint32_t width, uint32_t height) {
	vkb::SwapchainBuilder swapchainBuilder{chosenGpu, device, surface };
	
	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.set_composite_alpha_flags(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) //TODO??
		.build()
		.value();
	
	swapchainExtent = vkbSwapchain.extent;
	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Madline::GraphicsEngine::initSwapchain() {
	createSwapchain(windowExtent.width, windowExtent.height);
	
	//draw image size will match the mWindow
	VkExtent3D drawImageExtent = {
		windowExtent.width,
		windowExtent.height,
		1
	};
	
	//hardcoding the draw format to 32-bit float
	drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	drawImage.imageExtent = drawImageExtent;
	
	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rImgInfo = VkInit::imageCreateInfo(drawImage.imageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rImgAllocInfo = {};
	rImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rImgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator, &rImgInfo, &rImgAllocInfo, &drawImage.image, &drawImage.allocation, nullptr);

	//build an image-view for the draw image to use for rendering
	VkImageViewCreateInfo rViewInfo = VkInit::imageviewCreateInfo(drawImage.imageFormat, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(device, &rViewInfo, nullptr, &drawImage.imageView));

	//add to deletion queues
	mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyImageView(device, drawImage.imageView, nullptr);
		vmaDestroyImage(allocator, drawImage.image, drawImage.allocation);
	});
}

void Madline::GraphicsEngine::initCommands() {
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
	
	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &immCommandPool));
	
	// allocate the command buffer for immediate submits
	VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(immCommandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &immCommandBuffer));

	mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyCommandPool(device, immCommandPool, nullptr);
	});
}
void Madline::GraphicsEngine::initSyncStructures() {
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
	
	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &immFence));
	mainDeletionQueue.pushFunction([=, this]() { vkDestroyFence(device, immFence, nullptr); });
}

void Madline::GraphicsEngine::initDescriptors() {
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
    {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };
	
	globalDescriptorAllocator.initPool(device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		drawImageDescriptorLayout = builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);
	}
	
	//allocate a descriptor set for our draw image
	drawImageDescriptors = globalDescriptorAllocator.allocate(device,drawImageDescriptorLayout);
	
	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = drawImage.imageView;
	
	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = drawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);

	//make sure both the descriptor allocator and the new layout get cleaned up properly
	mainDeletionQueue.pushFunction([&]() {
		globalDescriptorAllocator.destroyPool(device);

		vkDestroyDescriptorSetLayout(device, drawImageDescriptorLayout, nullptr);
	});
}

void Madline::GraphicsEngine::initPipelines() {
	initBackgroundPipelines();
}

// add push constants if necessary
void Madline::GraphicsEngine::initBackgroundPipelines() {
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;
	
	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ComputePushConstants) ;
	pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	computeLayout.pPushConstantRanges = &pushConstant;
	computeLayout.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(device, &computeLayout, nullptr, &gradientPipelineLayout));
	
	VkPipelineShaderStageCreateInfo stageInfo{};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.pNext = nullptr;
	stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageInfo;
	
	const std::string gradientShaderName = "gradient_colour";
	ComputePushConstants pcGradient{};
	pcGradient.data1 = static_cast<glm::vec4>(Colour(255, 0, 0, 255));
	pcGradient.data2 = static_cast<glm::vec4>(Colour(0, 0, 255, 255));
	
	createComputeShader(gradientShaderName, pcGradient, &stageInfo, &computePipelineCreateInfo);
	computePipelineCreateInfo.stage = stageInfo;
	
	const std::string skyShaderName = "sky";
	ComputePushConstants pcSky{};
	pcSky.data1 = glm::vec4(0.1, 0.2, 0.4 ,0.97);
	
	createComputeShader(skyShaderName, pcSky, &stageInfo, &computePipelineCreateInfo);
}

void Madline::GraphicsEngine::initImgui() {
	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool));

	// 2: initialize imgui library

	// this initializes the core structures of imgui
	ImGui::CreateContext();

	// this initializes imgui for Win32
	ImGui_ImplWin32_Init(mWindow->getHwnd());

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = instance;
	initInfo.PhysicalDevice = chosenGpu;
	initInfo.Device = device;
	initInfo.Queue = graphicsQueue;
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = 3;
	initInfo.UseDynamicRendering = true;

	//dynamic rendering parameters for imgui to use
	initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
	initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;


	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&initInfo);

	ImGui_ImplVulkan_CreateFontsTexture();

	// add the deletor for the imgui created structures
	mainDeletionQueue.pushFunction([=, this]() {
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(device, imguiPool, nullptr);
	});
}

void Madline::GraphicsEngine::createComputeShader(
    const std::string& shaderName, const ComputePushConstants& pushConstants,
    VkPipelineShaderStageCreateInfo* stageInfo, VkComputePipelineCreateInfo* computePipelineCreateInfo
) {
	VkShaderModule computeDrawShader{};
	std::string shaderPath = std::format("{}/spv/{}.comp.spv", SHADER_PATH, shaderName);
	
	if (!VkUtil::loadShaderModule(shaderPath.c_str(), device, &computeDrawShader)) {
		std::printf("Error when building the compute shader\n");
	}
	
	stageInfo->module = computeDrawShader;
	computePipelineCreateInfo->stage = *stageInfo;
	
	ComputeEffect shader;
	shader.layout = gradientPipelineLayout;
	shader.name = shaderName;
	shader.data = pushConstants;
	
	VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, computePipelineCreateInfo, nullptr, &shader.pipeline));
	
	backgroundEffects.push_back(shader);
	
	vkDestroyShaderModule(device, computeDrawShader, nullptr);
	mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyPipelineLayout(device, gradientPipelineLayout, nullptr);
	});
}