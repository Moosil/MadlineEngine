//
// Created by School on 2025/3/14.
//

#include <chrono>
#include <format>
#include <iostream>
#include <ranges>
#include <thread>

#include "MadlineEngineDefines.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <vk_mem_alloc.h>

#include "winapi/game_window.h"

#include "vulkan/vk_images.h"
#include "vulkan/vk_initialisers.h"
#include "vulkan/vk_pipelines.h"

#include "vulkan/vk_engine.h"


Madline::GraphicsEngine *loadedEngine = nullptr;
#if BUILD_TYPE == BUILD_DEBUG
constexpr bool USE_VALIDATION_LAYERS = true;
#else
constexpr bool USE_VALIDATION_LAYERS = false;
#endif

Madline::GraphicsEngine::GraphicsEngine(Madline::Window &pWindow) {
	init(pWindow);
}

Madline::GraphicsEngine::~GraphicsEngine() {
	cleanup();
}

void Madline::GraphicsEngine::init(Madline::Window &pWindow) {
	std::printf("Started Vulkan initialisation\n");

	// only one engine initialization is allowed with the application.
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	mWindow = &pWindow;

	initVulkan(pWindow);

	const Rect2<int> screenRect = pWindow.getWindowRect();
	windowExtent.height = screenRect.getHeight();
	windowExtent.width = screenRect.getWidth();

	for (auto &[_, surfComp]: surfaces) {
		initSwapchain(surfComp);

		initCommands(surfComp);

		initSyncStructures(surfComp);

		initDescriptors(surfComp);

		initPipelines(surfComp);

		if (surfComp.hasImgui)
			initImgui(surfComp);
	}

	isInitialized = true;
	std::printf("Finished Vulkan initialisation\n");
}

void Madline::GraphicsEngine::cleanup() {
	if (isInitialized) {

		//make sure the gpu has stopped doing its things
		vkDeviceWaitIdle(device);

		//free per-frame structures and deletion queue
		for (auto &[_, surfComp]: surfaces) {
			for (auto &frame: surfComp.frames) {

				vkDestroyCommandPool(device, frame.commandPool, nullptr);

				//destroy sync objects
				vkDestroyFence(device, frame.renderFence, nullptr);
				vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
				vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);

				frame.deletionQueue.flush();
			}

			//flush the global deletion queue
			surfComp.mainDeletionQueue.flush();

			destroySwapchain(surfComp);

			vkDestroySurfaceKHR(instance, surfComp.surface, nullptr);
		}
		vkDestroyDevice(device, nullptr);

		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);
		if (mWindow) {
			delete (mWindow);
			mWindow = nullptr;
		}
	}

	// clear engine pointer
	loadedEngine = nullptr;
}

void Madline::GraphicsEngine::drawLoop() {
	// imgui new frame
	for (auto &[_, imguiHolder]: surfaces) {
		if (imguiHolder.hasImgui) {
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			for (auto &[_, surfComp]: surfaces) {
				if (surfComp.createImguiStuff != nullptr) {
					surfComp.createImguiStuff(surfComp);
				}
			}

			ImGui::End();

			ImGui::Render();
		}
	}

	//our draw function
	for (auto &[_, surfComp]: surfaces)
		draw(surfComp);
}

void Madline::GraphicsEngine::mainWindowCreateImgui(SurfaceComponents &surfComp) {
	if (ImGui::Begin("background")) {

		ComputeEffect &selected = surfComp.backgroundEffects[surfComp.currentBackgroundEffect];

		ImGui::Text("Selected effect: ", selected.name.c_str());

		ImGui::SliderInt("Effect Index", &surfComp.currentBackgroundEffect, 0,
		                 static_cast<int>(surfComp.backgroundEffects.size() - 1));

		ImGui::InputFloat4("data1", (float *) &selected.data.data1);
		ImGui::InputFloat4("data2", (float *) &selected.data.data2);
		ImGui::InputFloat4("data3", (float *) &selected.data.data3);
		ImGui::InputFloat4("data4", (float *) &selected.data.data4);
	}
}

void Madline::GraphicsEngine::drawBackground(VkCommandBuffer cmd, SurfaceComponents &surfComp) const {
	ComputeEffect effect = surfComp.backgroundEffects[surfComp.currentBackgroundEffect];

	// bind the background compute pipeline
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);

	// bind the descriptor set containing the draw image for the compute pipeline
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, surfComp.gradientPipelineLayout, 0, 1,
	                        &surfComp.drawImageDescriptors, 0, nullptr);

	vkCmdPushConstants(cmd, surfComp.gradientPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
	// Execute the compute pipeline dispatch. We are using 16x16 workgroup size, so we need to divide by it
	vkCmdDispatch(cmd, std::ceil(static_cast<float>(surfComp.drawExtent.width) / 16.0),
	              std::ceil(static_cast<float>(surfComp.drawExtent.height) / 16.0), 1);
}

void Madline::GraphicsEngine::drawImgui(VkCommandBuffer cmd, VkImageView targetImageView, SurfaceComponents &surfComp) {
	VkRenderingAttachmentInfo colorAttachment = VkInit::attachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = VkInit::renderingInfo(surfComp.swapchainExtent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}


void Madline::GraphicsEngine::draw(SurfaceComponents &surfComp) {
#pragma region Flush deletion queue
	//Wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &surfComp.getCurrentFrame().renderFence, true, 1'000'000'000));

	surfComp.getCurrentFrame().deletionQueue.flush();
#pragma endregion

#pragma region Draw 1
	// Wait until the gpu has finished rendering the last frame. Timeout of 1
	// second
	VK_CHECK(vkWaitForFences(device, 1, &surfComp.getCurrentFrame().renderFence, VK_TRUE, 1'000'000'000u));
	VK_CHECK(vkResetFences(device, 1, &surfComp.getCurrentFrame().renderFence));
#pragma endregion

#pragma region Draw 2
	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(
	        device, surfComp.swapchain, 1'000'000'000, surfComp.getCurrentFrame().swapchainSemaphore,
	        nullptr, &swapchainImageIndex));
#pragma endregion

#pragma region Draw 3
	//naming it cmd for shorter writing
	VkCommandBuffer cmd = surfComp.getCurrentFrame().mainCommandBuffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	//Begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

#pragma region Record command buffer
	surfComp.drawExtent.width = surfComp.drawImage.imageExtent.width;
	surfComp.drawExtent.height = surfComp.drawImage.imageExtent.height;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// transition our main draw image into general layout, so we can write into it,
	// we will overwrite it all, so we don't care about what was the older layout
	VkUtil::transitionImage(cmd, surfComp.drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	
	if (surfComp.drawingEnabled)
		drawBackground(cmd, surfComp);

	//transition the draw image and the swapchain image into their correct transfer layouts
	VkUtil::transitionImage(cmd, surfComp.drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VkUtil::transitionImage(cmd, surfComp.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	VkUtil::copyImageToImage(cmd, surfComp.drawImage.image, surfComp.swapchainImages[swapchainImageIndex], surfComp.drawExtent, surfComp.swapchainExtent);

	// set swapchain image layout to Attachment Optimal, so we can draw it
	VkUtil::transitionImage(cmd, surfComp.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
#pragma endregion

#pragma region Draw Imgui

	//draw imgui into the swapchain image
	if (surfComp.hasImgui)
		drawImgui(cmd, surfComp.swapchainImageViews[swapchainImageIndex], surfComp);

	// set swapchain image layout to Present, so we can draw it
	VkUtil::transitionImage(cmd, surfComp.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));
#pragma endregion

#pragma region Draw 5
	//Prepare the submission to the queue.
	//We want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkCommandBufferSubmitInfo cmdInfo = VkInit::commandBufferSubmitInfo(cmd);

	VkSemaphoreSubmitInfo waitInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, surfComp.getCurrentFrame().swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = VkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, surfComp.getCurrentFrame().renderSemaphore);

	VkSubmitInfo2 submit = VkInit::submitInfo(&cmdInfo, &signalInfo, &waitInfo);

	//Submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(surfComp.graphicsQueue, 1, &submit, surfComp.getCurrentFrame().renderFence));
#pragma endregion

#pragma region Draw 6
	//Prepare present
	// this will put the image we just rendered to into the visible mWindow.
	// We want to wait on the _renderSemaphore for that,
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &surfComp.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &surfComp.getCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(surfComp.graphicsQueue, &presentInfo));

	//increase the number of frames drawn
	surfComp.frameNumber++;
#pragma endregion
}

void Madline::GraphicsEngine::immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function,
                                              SurfaceComponents &surfComp) {
	VK_CHECK(vkResetFences(device, 1, &surfComp.immFence));
	VK_CHECK(vkResetCommandBuffer(surfComp.immCommandBuffer, 0));

	VkCommandBuffer cmd = surfComp.immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = VkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = VkInit::commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = VkInit::submitInfo(&cmdinfo, nullptr, nullptr);

	// Submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(surfComp.graphicsQueue, 1, &submit, surfComp.immFence));

	VK_CHECK(vkWaitForFences(device, 1, &surfComp.immFence, true, 9999999999));
}


void Madline::GraphicsEngine::initVulkan(const Madline::Window &window) {

	// Create VkBootstrap InstanceBuilder
	vkb::InstanceBuilder instanceBuilder;
	auto instanceRet = instanceBuilder
	                           .set_app_name(GAME_NAME)
	                           .set_app_version(1, 0, 0)
	                           .set_engine_name("MadlineEngine")
	                           .set_engine_version(1, 0, 0)
	                           .require_api_version(1, 3, 0)
	                           .request_validation_layers(USE_VALIDATION_LAYERS)
	                           .build();// build is always called last

	// Check if instance is null
	if (!instanceRet) {
		throw std::runtime_error(std::format("Failed to create Vulkan instance. Error: {}", instanceRet.error().message()));
	}

	// Get the built instance
	const vkb::Instance vkbInstance = instanceRet.value();

	instance = vkbInstance.instance;
	debugMessenger = vkbInstance.debug_messenger;

	// Create Windows api surface for Vulkan. If surface is non created successfully, throw a runtime error
	for (auto &[surfName, surfInfo]: window.getVulkanSurfaces(instance)) {
		SurfaceComponents component{};
		component.surface = surfInfo.surface;
		component.handle = surfInfo.handle;
		surfaces[surfName] = component;
	}

	surfaces[MAIN_WINDOW].createImguiStuff = mainWindowCreateImgui;
	surfaces[MAIN_WINDOW].drawingEnabled = true;
	surfaces[MAIN_WINDOW].hasImgui = true;
	

	//vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	features.dynamicRendering = true;
	features.synchronization2 = true;

	//vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	//Use vkbootstrap to select a gpu.
	//We want a gpu that can write to the WinAPI surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{vkbInstance};
	vkb::PhysicalDevice physicalDevice = selector
	                                             .set_minimum_version(1, 3)
	                                             .set_required_features_13(features)
	                                             .set_required_features_12(features12)
	                                             .set_surface(surfaces[MAIN_WINDOW].surface)
	                                             .select()
	                                             .value();

	//create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	device = vkbDevice.device;
	chosenGpu = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	for (auto &[_, surfComp]: surfaces) {
		surfComp.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		surfComp.graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

#pragma region VMA init
		// initialize the memory allocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = chosenGpu;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &surfComp.allocator);

		surfComp.mainDeletionQueue.pushFunction([&]() {
			vmaDestroyAllocator(surfComp.allocator);
		});
#pragma endregion
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surfaces[MAIN_WINDOW].surface, &surfaceCapabilities);
	std::printf("supported composite alpha: %u\n", surfaceCapabilities.supportedCompositeAlpha);
}

void Madline::GraphicsEngine::destroySwapchain(SurfaceComponents &surfComp) {
	vkDestroySwapchainKHR(device, surfComp.swapchain, nullptr);

	// destroy swapchain resources
	for (auto &swapchainImageView: surfComp.swapchainImageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}
}

void Madline::GraphicsEngine::createSwapchain(uint32_t width, uint32_t height, SurfaceComponents &surfComp) {
	vkb::SwapchainBuilder swapchainBuilder{chosenGpu, device, surfComp.surface};

	surfComp.swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
	                                      //.use_default_format_selection()
	                                      .set_desired_format(VkSurfaceFormatKHR{.format = surfComp.swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
	                                      //use vsync present mode
	                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	                                      .set_desired_extent(width, height)
	                                      .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	                                      .set_composite_alpha_flags(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)//TODO??
	                                      .build()
	                                      .value();

	surfComp.swapchainExtent = vkbSwapchain.extent;
	//store swapchain and its related images
	surfComp.swapchain = vkbSwapchain.swapchain;
	surfComp.swapchainImages = vkbSwapchain.get_images().value();
	surfComp.swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Madline::GraphicsEngine::initSwapchain(SurfaceComponents &surfComp) {
	createSwapchain(windowExtent.width, windowExtent.height, surfComp);

	//draw image size will match the mWindow
	VkExtent3D drawImageExtent = {
	        windowExtent.width,
	        windowExtent.height,
	        1};

	//hardcoding the draw format to 32-bit float
	surfComp.drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	surfComp.drawImage.imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rImgInfo = VkInit::imageCreateInfo(surfComp.drawImage.imageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rImgAllocInfo = {};
	rImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rImgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(surfComp.allocator, &rImgInfo, &rImgAllocInfo, &surfComp.drawImage.image, &surfComp.drawImage.allocation, nullptr);

	//build an image-view for the draw image to use for rendering
	VkImageViewCreateInfo rViewInfo = VkInit::imageviewCreateInfo(surfComp.drawImage.imageFormat, surfComp.drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(device, &rViewInfo, nullptr, &surfComp.drawImage.imageView));

	//add to deletion queues
	surfComp.mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyImageView(device, surfComp.drawImage.imageView, nullptr);
		vmaDestroyImage(surfComp.allocator, surfComp.drawImage.image, surfComp.drawImage.allocation);
	});
}

void Madline::GraphicsEngine::initCommands(SurfaceComponents &surfComp) {
	//Create a command pool for commands submitted to the graphics queue.
	//We also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = VkInit::commandPoolCreateInfo(
	        surfComp.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (auto &frame: surfComp.frames) {

		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frame.commandPool));

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(frame.commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frame.mainCommandBuffer));
	}

	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &surfComp.immCommandPool));

	// allocate the command buffer for immediate submits
	VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::commandBufferAllocateInfo(surfComp.immCommandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &surfComp.immCommandBuffer));

	surfComp.mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyCommandPool(device, surfComp.immCommandPool, nullptr);
	});
}
void Madline::GraphicsEngine::initSyncStructures(SurfaceComponents &surfComp) {
	//create synchronisation structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to synchronise rendering with swapchain
	//we want the fence to start signalled, so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = VkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = VkInit::semaphoreCreateInfo();

	for (auto &frame: surfComp.frames) {
		VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.renderFence));

		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSemaphore));
	}

	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &surfComp.immFence));
	surfComp.mainDeletionQueue.pushFunction([=, this]() { vkDestroyFence(device, surfComp.immFence, nullptr); });
}

void Madline::GraphicsEngine::initDescriptors(SurfaceComponents &surfComp) {
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
	        {
	                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

	surfComp.globalDescriptorAllocator.initPool(device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		surfComp.drawImageDescriptorLayout = builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	//allocate a descriptor set for our draw image
	surfComp.drawImageDescriptors = surfComp.globalDescriptorAllocator.allocate(device, surfComp.drawImageDescriptorLayout);

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = surfComp.drawImage.imageView;

	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;

	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = surfComp.drawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);

	//make sure both the descriptor allocator and the new layout get cleaned up properly
	surfComp.mainDeletionQueue.pushFunction([&]() {
		surfComp.globalDescriptorAllocator.destroyPool(device);

		vkDestroyDescriptorSetLayout(device, surfComp.drawImageDescriptorLayout, nullptr);
	});
}

void Madline::GraphicsEngine::initPipelines(SurfaceComponents &surfComp) {
	initBackgroundPipelines(surfComp);
}

// add push constants if necessary
void Madline::GraphicsEngine::initBackgroundPipelines(SurfaceComponents &surfComp) {
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &surfComp.drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(ComputePushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	computeLayout.pPushConstantRanges = &pushConstant;
	computeLayout.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(device, &computeLayout, nullptr, &surfComp.gradientPipelineLayout));

	VkPipelineShaderStageCreateInfo stageInfo{};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.pNext = nullptr;
	stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = surfComp.gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageInfo;

	const std::string gradientShaderName = "gradient_colour";
	ComputePushConstants pcGradient{};
	pcGradient.data1 = static_cast<glm::vec4>(Colour(255, 0, 0, 255));
	pcGradient.data2 = static_cast<glm::vec4>(Colour(0, 0, 255, 255));

	createComputeShader(gradientShaderName, pcGradient, &stageInfo, &computePipelineCreateInfo, surfComp);
	computePipelineCreateInfo.stage = stageInfo;

	const std::string skyShaderName = "sky";
	ComputePushConstants pcSky{};
	pcSky.data1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

	createComputeShader(skyShaderName, pcSky, &stageInfo, &computePipelineCreateInfo, surfComp);
}

void Madline::GraphicsEngine::initImgui(SurfaceComponents &surfComp) {
	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
	                                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
	                                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
	                                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
	                                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
	                                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = (uint32_t) std::size(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool));

// 2: initialize imgui library

// Checks if imgui files are valid
#if BUILD_TYPE == BUILD_DEBUG
	IMGUI_CHECKVERSION();
#endif

	// this initializes the core structures of imgui
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls

	// this initializes imgui for Win32
	ImGui_ImplWin32_Init(surfComp.handle);

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = instance;
	initInfo.PhysicalDevice = chosenGpu;
	initInfo.Device = device;
	initInfo.Queue = surfComp.graphicsQueue;
	initInfo.DescriptorPool = imguiPool;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = 3;
	initInfo.UseDynamicRendering = true;

	//dynamic rendering parameters for imgui to use
	initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
	initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &surfComp.swapchainImageFormat;


	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&initInfo);

	ImGui_ImplVulkan_CreateFontsTexture();

	// add the deletor for the imgui created structures
	surfComp.mainDeletionQueue.pushFunction([=, this]() {
		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(device, imguiPool, nullptr);
	});
}

void Madline::GraphicsEngine::createComputeShader(const std::string &shaderName,
                                                  const ComputePushConstants &pushConstants,
                                                  VkPipelineShaderStageCreateInfo *stageInfo,
                                                  VkComputePipelineCreateInfo *computePipelineCreateInfo,
                                                  SurfaceComponents &surfComp) {
	VkShaderModule computeDrawShader{};
	std::string shaderPath = std::format("{}/spv/{}.comp.spv", SHADER_PATH, shaderName);

	if (!VkUtil::loadShaderModule(shaderPath.c_str(), device, &computeDrawShader)) {
		std::printf("Error when building the compute shader\n");
	}

	stageInfo->module = computeDrawShader;
	computePipelineCreateInfo->stage = *stageInfo;

	ComputeEffect shader;
	shader.layout = surfComp.gradientPipelineLayout;
	shader.name = shaderName;
	shader.data = pushConstants;

	VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, computePipelineCreateInfo, nullptr, &shader.pipeline));

	surfComp.backgroundEffects.push_back(shader);

	vkDestroyShaderModule(device, computeDrawShader, nullptr);
	surfComp.mainDeletionQueue.pushFunction([=, this]() {
		vkDestroyPipelineLayout(device, surfComp.gradientPipelineLayout, nullptr);
	});
}