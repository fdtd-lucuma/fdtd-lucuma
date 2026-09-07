// No direct Julia counterpart — compute-pipeline helper (mirrors the role of
// fdtd-lucuma/src/services/vulkan/compute.cppm ComputePipeline).
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include "fdtd/backends/vulkan/context.hpp"

namespace lucuma::vulkan::vk {

// One compute shader with `nBuffers` storage buffers at bindings 0..n-1 and a
// single push-constant block of `pushBytes`.
class ComputePipeline {
public:
	ComputePipeline(Context& ctx, std::span<const uint32_t> spirv, int nBuffers,
	                uint32_t pushBytes, const char* entry = "main");
	~ComputePipeline();
	ComputePipeline(const ComputePipeline&) = delete;
	ComputePipeline& operator=(const ComputePipeline&) = delete;

	// Point the descriptor set at these buffers (order == binding index).
	void bindBuffers(std::span<const VkBuffer> buffers);

	// Bind + push + dispatch into an open command buffer.
	void dispatch(VkCommandBuffer cb, const void* push, uint32_t pushBytes,
	              uint32_t gx, uint32_t gy = 1, uint32_t gz = 1);

private:
	Context& ctx_;
	int nBuffers_;
	VkShaderModule module_ = VK_NULL_HANDLE;
	VkDescriptorSetLayout setLayout_ = VK_NULL_HANDLE;
	VkPipelineLayout layout_ = VK_NULL_HANDLE;
	VkPipeline pipeline_ = VK_NULL_HANDLE;
	VkDescriptorPool pool_ = VK_NULL_HANDLE;
	VkDescriptorSet set_ = VK_NULL_HANDLE;
};

}  // namespace lucuma::vulkan::vk
