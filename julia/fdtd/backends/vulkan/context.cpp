// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/backends/vulkan/context.hpp"

#include <cstring>
#include <stdexcept>

namespace lucuma::julia::vk {

const char* resultString(VkResult r) {
	switch (r) {
		case VK_SUCCESS:                     return "VK_SUCCESS";
		case VK_ERROR_OUT_OF_HOST_MEMORY:    return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:  return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:   return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:   return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_DEVICE_LOST:           return "VK_ERROR_DEVICE_LOST";
		default:                             return "VK_ERROR_<other>";
	}
}

static void check(VkResult r, const char* what) {
	if (r != VK_SUCCESS)
		throw std::runtime_error(std::string("vulkan: ") + what + " -> " +
		                         resultString(r));
}

Context::Context() {
	// ---- instance (portability enumeration, only if the loader advertises it) ----
	VkApplicationInfo app{};
	app.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.apiVersion = VK_API_VERSION_1_1;

	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	std::vector<VkExtensionProperties> exts(extCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, exts.data());
	bool hasPortability = false;
	for (const auto& e : exts)
		if (std::strcmp(e.extensionName,
		                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
			hasPortability = true;

	std::vector<const char*> instExts;
	VkInstanceCreateInfo ici{};
	ici.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ici.pApplicationInfo = &app;
	if (hasPortability) {
		instExts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		ici.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	}
	ici.enabledExtensionCount   = static_cast<uint32_t>(instExts.size());
	ici.ppEnabledExtensionNames = instExts.data();
	check(vkCreateInstance(&ici, nullptr, &instance_), "vkCreateInstance");

	// ---- physical device: first one with a compute queue ----
	uint32_t nDev = 0;
	vkEnumeratePhysicalDevices(instance_, &nDev, nullptr);
	if (nDev == 0) throw std::runtime_error("vulkan: no physical devices");
	std::vector<VkPhysicalDevice> devs(nDev);
	vkEnumeratePhysicalDevices(instance_, &nDev, devs.data());

	bool found = false;
	for (auto d : devs) {
		uint32_t nQ = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(d, &nQ, nullptr);
		std::vector<VkQueueFamilyProperties> qf(nQ);
		vkGetPhysicalDeviceQueueFamilyProperties(d, &nQ, qf.data());
		for (uint32_t i = 0; i < nQ; ++i)
			if (qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				phys_   = d;
				family_ = i;
				found   = true;
				break;
			}
		if (found) break;
	}
	if (!found) throw std::runtime_error("vulkan: no compute queue family");

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(phys_, &props);
	deviceName_ = props.deviceName;
	vkGetPhysicalDeviceMemoryProperties(phys_, &memProps_);

	// ---- logical device ----
	uint32_t devExtCount = 0;
	vkEnumerateDeviceExtensionProperties(phys_, nullptr, &devExtCount, nullptr);
	std::vector<VkExtensionProperties> devExts(devExtCount);
	vkEnumerateDeviceExtensionProperties(phys_, nullptr, &devExtCount,
	                                     devExts.data());
	std::vector<const char*> wantExts;
	for (const auto& e : devExts)
		if (std::strcmp(e.extensionName, "VK_KHR_portability_subset") == 0)
			wantExts.push_back("VK_KHR_portability_subset");

	float prio = 1.0f;
	VkDeviceQueueCreateInfo qci{};
	qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qci.queueFamilyIndex = family_;
	qci.queueCount       = 1;
	qci.pQueuePriorities = &prio;

	VkDeviceCreateInfo dci{};
	dci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.queueCreateInfoCount    = 1;
	dci.pQueueCreateInfos       = &qci;
	dci.enabledExtensionCount   = static_cast<uint32_t>(wantExts.size());
	dci.ppEnabledExtensionNames = wantExts.data();
	check(vkCreateDevice(phys_, &dci, nullptr, &device_), "vkCreateDevice");
	vkGetDeviceQueue(device_, family_, 0, &queue_);

	VkCommandPoolCreateInfo pci{};
	pci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pci.queueFamilyIndex = family_;
	check(vkCreateCommandPool(device_, &pci, nullptr, &pool_),
	      "vkCreateCommandPool");
}

Context::~Context() {
	if (device_) vkDeviceWaitIdle(device_);
	if (pool_)   vkDestroyCommandPool(device_, pool_, nullptr);
	if (device_) vkDestroyDevice(device_, nullptr);
	if (instance_) vkDestroyInstance(instance_, nullptr);
}

uint32_t Context::findMemoryTypeSafe(uint32_t typeBits,
                                     VkMemoryPropertyFlags props) const noexcept {
	for (uint32_t i = 0; i < memProps_.memoryTypeCount; ++i)
		if ((typeBits & (1u << i)) &&
		    (memProps_.memoryTypes[i].propertyFlags & props) == props)
			return i;
	return UINT32_MAX;
}

uint32_t Context::findMemoryType(uint32_t typeBits,
                                 VkMemoryPropertyFlags props) const {
	uint32_t idx = findMemoryTypeSafe(typeBits, props);
	if (idx == UINT32_MAX)
		throw std::runtime_error("vulkan: no suitable memory type");
	return idx;
}

Buffer Context::createBuffer(VkDeviceSize bytes, bool preferDeviceLocal) {
	Buffer b;
	b.size = bytes;

	VkBufferCreateInfo bci{};
	bci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size        = bytes;
	bci.usage       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
	                  VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	check(vkCreateBuffer(device_, &bci, nullptr, &b.buffer), "vkCreateBuffer");

	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(device_, b.buffer, &req);

	VkMemoryAllocateInfo mai{};
	mai.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = req.size;

	bool hostVisible = false;

	if (preferDeviceLocal) {
		// Priority 1: DEVICE_LOCAL + HOST_VISIBLE + HOST_COHERENT
		//   → Metal shared on Apple Silicon (UMA): zero-copy, same DRAM.
		//   → discrete GPU with resizable BAR: whole VRAM, also zero-copy.
		// Reject it when it lives in a small heap (the 256 MB PCIe BAR window on
		// a discrete GPU without ReBAR) — there it OOMs; use pure VRAM instead.
		uint32_t idx = findMemoryTypeSafe(
		    req.memoryTypeBits,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  |
		    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (idx != UINT32_MAX) {
			const VkDeviceSize heap =
			    memProps_.memoryHeaps[memProps_.memoryTypes[idx].heapIndex].size;
			if (heap < (VkDeviceSize{1} << 30)) idx = UINT32_MAX;  // < 1 GiB
		}

		if (idx != UINT32_MAX) {
			mai.memoryTypeIndex = idx;
			hostVisible = true;
		} else {
			// Priority 2: pure DEVICE_LOCAL (discrete GPU VRAM — staging needed)
			idx = findMemoryTypeSafe(req.memoryTypeBits,
			                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			if (idx != UINT32_MAX) {
				mai.memoryTypeIndex = idx;
				hostVisible = false;
			} else {
				// Fallback: any host-visible memory
				mai.memoryTypeIndex = findMemoryType(
				    req.memoryTypeBits,
				    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				hostVisible = true;
			}
		}
	} else {
		// Staging / readback buffer: HOST_VISIBLE + HOST_COHERENT
		mai.memoryTypeIndex = findMemoryType(
		    req.memoryTypeBits,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		hostVisible = true;
	}

	check(vkAllocateMemory(device_, &mai, nullptr, &b.memory),
	      "vkAllocateMemory");
	check(vkBindBufferMemory(device_, b.buffer, b.memory, 0),
	      "vkBindBufferMemory");

	if (hostVisible) {
		check(vkMapMemory(device_, b.memory, 0, VK_WHOLE_SIZE, 0, &b.mapped),
		      "vkMapMemory");
		std::memset(b.mapped, 0, static_cast<size_t>(bytes));
	}
	// b.mapped == nullptr for pure-DEVICE_LOCAL allocations

	return b;
}

void Context::destroy(Buffer& b) {
	if (b.mapped)  vkUnmapMemory(device_, b.memory);
	if (b.buffer)  vkDestroyBuffer(device_, b.buffer, nullptr);
	if (b.memory)  vkFreeMemory(device_, b.memory, nullptr);
	b = {};
}

// ---------------------------------------------------------------------------
// Transfer helpers

void Context::uploadToDevice(Buffer& dst, const void* src, VkDeviceSize bytes,
                             VkDeviceSize dstOffset) {
	if (dst.mapped) {
		// HOST_VISIBLE path (UMA / Metal shared): direct memcpy, no GPU work.
		std::memcpy(static_cast<char*>(dst.mapped) + dstOffset, src,
		            static_cast<size_t>(bytes));
		return;
	}
	// Pure DEVICE_LOCAL: stage through a transient HOST_VISIBLE buffer.
	Buffer stg = createBuffer(bytes, /*preferDeviceLocal=*/false);
	std::memcpy(stg.mapped, src, static_cast<size_t>(bytes));
	submitSync([&](VkCommandBuffer cb) {
		recordCopy(cb, stg.buffer, dst.buffer, bytes, 0, dstOffset);
	});
	destroy(stg);
}

void Context::downloadFromDevice(const Buffer& src, void* dst,
                                 VkDeviceSize bytes, VkDeviceSize srcOffset) {
	if (src.mapped) {
		std::memcpy(dst,
		            static_cast<const char*>(src.mapped) + srcOffset,
		            static_cast<size_t>(bytes));
		return;
	}
	Buffer stg = createBuffer(bytes, /*preferDeviceLocal=*/false);
	submitSync([&](VkCommandBuffer cb) {
		recordCopy(cb, src.buffer, stg.buffer, bytes, srcOffset, 0);
	});
	std::memcpy(dst, stg.mapped, static_cast<size_t>(bytes));
	destroy(stg);
}

void Context::fillBuffer(Buffer& b, uint32_t value) {
	if (b.mapped) {
		std::memset(b.mapped, static_cast<int>(value), static_cast<size_t>(b.size));
		return;
	}
	submitSync([&](VkCommandBuffer cb) { recordFill(cb, b, value); });
}

/*static*/ void Context::recordFill(VkCommandBuffer cb, const Buffer& b,
                                    uint32_t value) {
	vkCmdFillBuffer(cb, b.buffer, 0, VK_WHOLE_SIZE, value);
}

/*static*/ void Context::recordCopy(VkCommandBuffer cb, VkBuffer src,
                                    VkBuffer dst, VkDeviceSize size,
                                    VkDeviceSize srcOffset,
                                    VkDeviceSize dstOffset) {
	VkBufferCopy region{srcOffset, dstOffset, size};
	vkCmdCopyBuffer(cb, src, dst, 1, &region);
}

// ---------------------------------------------------------------------------

void Context::submitSync(const std::function<void(VkCommandBuffer)>& rec) {
	VkCommandBufferAllocateInfo cbai{};
	cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool        = pool_;
	cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;
	VkCommandBuffer cb;
	check(vkAllocateCommandBuffers(device_, &cbai, &cb),
	      "vkAllocateCommandBuffers");

	VkCommandBufferBeginInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	check(vkBeginCommandBuffer(cb, &bi), "vkBeginCommandBuffer");
	rec(cb);
	check(vkEndCommandBuffer(cb), "vkEndCommandBuffer");

	VkSubmitInfo si{};
	si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers    = &cb;
	check(vkQueueSubmit(queue_, 1, &si, VK_NULL_HANDLE), "vkQueueSubmit");
	check(vkQueueWaitIdle(queue_), "vkQueueWaitIdle");
	vkFreeCommandBuffers(device_, pool_, 1, &cb);
}

VkShaderModule Context::loadShader(std::span<const uint32_t> spirv) {
	VkShaderModuleCreateInfo smci{};
	smci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	smci.codeSize = spirv.size() * sizeof(uint32_t);
	smci.pCode    = spirv.data();
	VkShaderModule m;
	check(vkCreateShaderModule(device_, &smci, nullptr, &m),
	      "vkCreateShaderModule");
	return m;
}

void computeBarrier(VkCommandBuffer cb) {
	VkMemoryBarrier mb{};
	mb.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &mb, 0,
	                     nullptr, 0, nullptr);
}

}  // namespace lucuma::julia::vk
