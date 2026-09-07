// No direct Julia counterpart — minimal Vulkan compute context (mirrors the
// role of fdtd-lucuma/src/services/vulkan/{core,device,compute}.cppm).
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace lucuma::julia::vk {

// A host-visible, coherent SSBO (MoltenVK is UMA, so no staging is needed).
struct Buffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void* mapped = nullptr;
	VkDeviceSize size = 0;
};

class Context {
public:
	Context();
	~Context();
	Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;

	VkDevice device() const { return device_; }
	VkQueue queue() const { return queue_; }
	uint32_t queueFamily() const { return family_; }
	VkCommandPool commandPool() const { return pool_; }
	const std::string& deviceName() const { return deviceName_; }

	Buffer createBuffer(VkDeviceSize bytes);
	void destroy(Buffer& b);

	// Record `rec` into a primary command buffer, submit, wait idle.
	void submitSync(const std::function<void(VkCommandBuffer)>& rec);

	// Load a SPIR-V blob into a shader module.
	VkShaderModule loadShader(std::span<const uint32_t> spirv);

private:
	uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props) const;

	VkInstance instance_ = VK_NULL_HANDLE;
	VkPhysicalDevice phys_ = VK_NULL_HANDLE;
	VkDevice device_ = VK_NULL_HANDLE;
	VkQueue queue_ = VK_NULL_HANDLE;
	uint32_t family_ = 0;
	VkCommandPool pool_ = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties memProps_{};
	std::string deviceName_;
};

// Insert a compute->compute (shader write -> shader read) full memory barrier.
void computeBarrier(VkCommandBuffer cb);

const char* resultString(VkResult r);

}  // namespace lucuma::julia::vk
