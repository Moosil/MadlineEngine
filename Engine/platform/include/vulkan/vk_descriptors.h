//
// Created by School on 2025/3/18.
//

#ifndef CELESTEPET_VK_DESCRIPTORS_H
#define CELESTEPET_VK_DESCRIPTORS_H

#include <vector>

#include "vk_types.h"

#include <vulkan/vulkan.h>

namespace Madline {
	struct DescriptorLayoutBuilder {

		std::vector<VkDescriptorSetLayoutBinding> bindings;

		void addBinding(uint32_t binding, VkDescriptorType type);
		void clear();
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void *pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
	};
	
	struct DescriptorAllocator {
		
		struct PoolSizeRatio {
			VkDescriptorType type;
			float ratio;
		};

		VkDescriptorPool pool;

		void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void clearDescriptors(VkDevice device) const;
		void destroyPool(VkDevice device) const;

		VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout) const;
	};

}


#endif//CELESTEPET_VK_DESCRIPTORS_H
