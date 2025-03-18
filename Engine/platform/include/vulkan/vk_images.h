//
// Created by School on 2025/3/16.
//

#ifndef CELESTEPET_VK_IMAGES_H
#define CELESTEPET_VK_IMAGES_H

#include <vulkan/vulkan.h>

namespace VkUtil {
	void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
};

#endif//CELESTEPET_VK_IMAGES_H
