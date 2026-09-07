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

namespace lucuma::vulkan::vk {

// A Vulkan SSBO.
//
// `mapped != nullptr` means the buffer's backing memory is HOST_VISIBLE and
// CPU can read/write directly (e.g. Metal shared on Apple Silicon, or any
// host-visible memory on integrated/UMA GPUs).  In that case
// uploadToDevice / downloadFromDevice / fillBuffer all take the fast
// memcpy / memset path — no staging command buffers needed.
//
// `mapped == nullptr` means the buffer lives in pure DEVICE_LOCAL memory
// (e.g. Metal private on a discrete GPU).  Transfers then go through a
// transient staging buffer submitted with submitSync.
struct Buffer {
	VkBuffer       buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	void*          mapped = nullptr;   // non-null ↔ HOST_VISIBLE (zero-copy path)
	VkDeviceSize   size   = 0;
};

class Context {
public:
	Context();
	~Context();
	Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;

	VkDevice      device()      const { return device_; }
	VkQueue       queue()       const { return queue_; }
	uint32_t      queueFamily() const { return family_; }
	VkCommandPool commandPool() const { return pool_; }
	const std::string& deviceName() const { return deviceName_; }

	// Allocate an SSBO.
	//
	// preferDeviceLocal=true (default): tries to find memory that is good for
	// the GPU.  On UMA / Apple Silicon the preferred type is
	// DEVICE_LOCAL|HOST_VISIBLE|HOST_COHERENT (Metal shared — zero-copy,
	// fastest for both CPU and GPU).  On discrete GPUs it falls back to pure
	// DEVICE_LOCAL and uses staging for CPU transfers.
	//
	// preferDeviceLocal=false: allocates HOST_VISIBLE|HOST_COHERENT memory
	// (staging / readback buffers).
	Buffer createBuffer(VkDeviceSize bytes, bool preferDeviceLocal = true);
	void   destroy(Buffer& b);

	// CPU → GPU.  Fast memcpy when dst.mapped is non-null; staging upload
	// otherwise.  dstOffset is the byte offset within the destination buffer.
	void uploadToDevice(Buffer& dst, const void* src, VkDeviceSize bytes,
	                    VkDeviceSize dstOffset = 0);

	// GPU → CPU.  Fast memcpy when src.mapped is non-null; staging readback
	// otherwise.
	void downloadFromDevice(const Buffer& src, void* dst, VkDeviceSize bytes,
	                        VkDeviceSize srcOffset = 0);

	// Fill the entire buffer with `value`.  Uses memset when mapped, otherwise
	// records vkCmdFillBuffer and submits synchronously.
	void fillBuffer(Buffer& b, uint32_t value = 0);

	// Low-level: record vkCmdFillBuffer into an ALREADY-OPEN command buffer
	// without submitting.  Use inside your own submitSync to batch fills.
	static void recordFill(VkCommandBuffer cb, const Buffer& b,
	                       uint32_t value = 0);

	// Low-level: record vkCmdCopyBuffer into an already-open command buffer.
	static void recordCopy(VkCommandBuffer cb, VkBuffer src, VkBuffer dst,
	                       VkDeviceSize size, VkDeviceSize srcOffset = 0,
	                       VkDeviceSize dstOffset = 0);

	// Record `rec` into a one-shot primary command buffer, submit, wait idle.
	void submitSync(const std::function<void(VkCommandBuffer)>& rec);

	// Load a SPIR-V blob into a shader module.
	VkShaderModule loadShader(std::span<const uint32_t> spirv);

private:
	uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props) const;
	// Returns UINT32_MAX if no matching type exists (does not throw).
	uint32_t findMemoryTypeSafe(uint32_t typeBits,
	                            VkMemoryPropertyFlags props) const noexcept;

	VkInstance                       instance_   = VK_NULL_HANDLE;
	VkPhysicalDevice                 phys_       = VK_NULL_HANDLE;
	VkDevice                         device_     = VK_NULL_HANDLE;
	VkQueue                          queue_      = VK_NULL_HANDLE;
	uint32_t                         family_     = 0;
	VkCommandPool                    pool_       = VK_NULL_HANDLE;
	VkPhysicalDeviceMemoryProperties memProps_{};
	std::string                      deviceName_;
};

// Insert a compute→compute (shader write → shader read) full memory barrier.
void computeBarrier(VkCommandBuffer cb);

const char* resultString(VkResult r);

}  // namespace lucuma::vulkan::vk
