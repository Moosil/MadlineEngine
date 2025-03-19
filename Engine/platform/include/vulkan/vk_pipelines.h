//
// Created by School on 2025/3/15.
//

#ifndef CELESTEPET_VK_PIPELINES_H
#define CELESTEPET_VK_PIPELINES_H

#include <vulkan/vk_types.h>

namespace VkUtil {
	bool loadShaderModule(const char *filePath, VkDevice device, VkShaderModule *outShaderModule);
};

#endif//CELESTEPET_VK_PIPELINES_H
