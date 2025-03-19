//
// Created by School on 2025/3/16.
//

#ifndef CELESTEPET_VK_IMAGES_H
#define CELESTEPET_VK_IMAGES_H

#include <vulkan/vulkan.h>

namespace VkUtil {
	void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
	
	// TODO write your own version that can do extra logic on a fullscreen fragment shader.
	void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
};

#endif//CELESTEPET_VK_IMAGES_H
