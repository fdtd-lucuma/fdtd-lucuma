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

// A Vulkan SSBO.  When deviceLocal==true the buffer lives in VRAM and
// `mapped` is nullptr; use Context::uploadToDevice / downloadFromDevice.
// When deviceLocal==false (staging) the buffer is HOST_VISIBLE|COHERENT
// and `mapped` points to the persistently-mapped host memory.
struct Buffer {
	VkBuffer       buffer      = VK_NULL_HANDLE;
	VkDeviceMemory memory      = VK_NULL_HANDLE;
	void*          mapped      = nullptr;   // non-null only for staging buffers
	VkDeviceSize   size        = 0;
	bool           deviceLocal = false;
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

	// Allocate an SSBO.  Pass deviceLocal=true for field/coeff/DFT buffers
	// (VRAM), false for host-visible staging buffers.
	Buffer createBuffer(VkDeviceSize bytes, bool deviceLocal = true);
	void   destroy(Buffer& b);

	// Copy `bytes` from CPU memory into a device-local buffer via a transient
	// staging buffer.  The caller may pass fewer bytes than b.size to do a
	// partial upload starting at offset `dstOffset`.
	void uploadToDevice(Buffer& dst, const void* src, VkDeviceSize bytes,
	                    VkDeviceSize dstOffset = 0);

	// Copy `bytes` from a device-local buffer back to CPU memory.
	void downloadFromDevice(const Buffer& src, void* dst, VkDeviceSize bytes,
	                        VkDeviceSize srcOffset = 0);

	// Zero a device-local buffer (or any range) entirely on the GPU using
	// vkCmdFillBuffer — avoids a CPU→GPU memset over PCIe.
	void fillBuffer(Buffer& b, uint32_t value = 0);

	// Record `rec` into a primary command buffer, submit, wait idle.
	void submitSync(const std::function<void(VkCommandBuffer)>& rec);

	// Load a SPIR-V blob into a shader module.
	VkShaderModule loadShader(std::span<const uint32_t> spirv);

private:
	uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props) const;
	// Allocate a transient HOST_VISIBLE|COHERENT staging buffer, fill it from
	// `src`, and return it.  Caller must destroy it after use.
	Buffer makeStagingBuffer(const void* src, VkDeviceSize bytes);

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

}  // namespace lucuma::julia::vk
