//
// Created by School on 2025/3/15.
//

#ifndef CELESTEPET_VK_TYPES_H
#define CELESTEPET_VK_TYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>
#include <format>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <mat4x4.hpp>
#include <vec4.hpp>

// We will add our main reusable types here
struct AllocatedImage {
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
            std::printf("Detected Vulkan error: %s\n", string_VkResult(err)); \
            abort();                                                    \
        }                                                               \
    } while (0)
#endif//CELESTEPET_VK_TYPES_H
