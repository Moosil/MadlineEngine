//
// Created by School on 2025/3/18.
//

#include "vulkan/vk_descriptors.h"
#include "vulkan/vk_types.h"

void Madline::DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type) {
	VkDescriptorSetLayoutBinding newBind {};
	newBind.binding = binding;
	newBind.descriptorCount = 1;
	newBind.descriptorType = type;
	
	bindings.push_back(newBind);
}

void Madline::DescriptorLayoutBuilder::clear() {
	bindings.clear();
}

VkDescriptorSetLayout Madline::DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags) {
	for (auto& b : bindings) {
		b.stageFlags |= shaderStages;
	}
	
	VkDescriptorSetLayoutCreateInfo info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	info.pNext = pNext;

	info.pBindings = bindings.data();
	info.bindingCount = (uint32_t)bindings.size();
	info.flags = flags;

	VkDescriptorSetLayout set;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}

void Madline::DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * maxSets)
		});
	}
	
	VkDescriptorPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	poolInfo.flags = 0;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
}

void Madline::DescriptorAllocator::clearDescriptors(VkDevice device) const {
	vkResetDescriptorPool(device, pool, 0);
}

void Madline::DescriptorAllocator::destroyPool(VkDevice device) const {
	vkDestroyDescriptorPool(device,pool,nullptr);
}

VkDescriptorSet Madline::DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) const {
	VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;
	
	VkDescriptorSet ds;
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

	return ds;
}
