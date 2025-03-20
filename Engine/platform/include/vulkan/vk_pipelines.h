//
// Created by School on 2025/3/15.
//

#ifndef MADLINEENGINE_VK_PIPELINES_H
#define MADLINEENGINE_VK_PIPELINES_H

#include <vulkan/vk_types.h>

namespace VkUtil {
	std::vector<uint32_t> readSpirv(const std::string& filePath);
	bool loadShaderModule(const char *filePath, VkDevice device, VkShaderModule *outShaderModule);
}

#endif//MADLINEENGINE_VK_PIPELINES_H
