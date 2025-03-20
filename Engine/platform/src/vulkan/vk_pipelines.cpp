//
// Created by School on 2025/3/15.
//

#include "vulkan/vk_pipelines.h"
#include <fstream>
#include "vulkan/vk_initialisers.h"

std::vector<uint32_t> VkUtil::readSpirv(const std::string &filePath) {
	// Open the file. With cursor at the end
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filePath);
	}
	
	// find what the size of the file is by looking up the location of the cursor
	// because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = static_cast<size_t>(file.tellg());

	// Spirv expects the buffer to be on uint32, so make sure to reserve an int
	// vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// put file cursor at beginning
	file.seekg(0);

	// load the entire file into the buffer
	file.read((char*)buffer.data(), static_cast<std::streamsize>(fileSize));

	// now that the file is loaded into the buffer, we can close it
	file.close();
	
	return buffer;
}


bool VkUtil::loadShaderModule(const char* filePath,
    VkDevice device,
    VkShaderModule* outShaderModule
) {
	std::vector<uint32_t> buffer = readSpirv(filePath);

	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	// codeSize has to be in bytes, so multiply the ints in the buffer by size of
	// int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	// check that the creation goes well.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}
	*outShaderModule = shaderModule;
	return true;
}