// Una GUI para fdtd
// Copyright © 2025 Otreblan
//
// fdtd-vulkan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-vulkan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.

module;

module fdtd.services.vulkan;

import vk_mem_alloc_hpp;


VulkanBuffer::VulkanBuffer(VulkanBuffer&& other):
	buffer(std::exchange(other.buffer, {})),
	allocation(std::exchange(other.allocation, {}))
{}

vk::Buffer VulkanBuffer::getBuffer()
{
	return *buffer;
}

vma::AllocationInfo VulkanBuffer::getInfo()
{
	return info;
}

vma::Allocation VulkanBuffer::getAllocation()
{
	return *allocation;
}

VulkanAllocator::VulkanAllocator(Injector& injector):
	vulkanCore(injector.inject<VulkanCore>()),
	vulkanDevice(injector.inject<VulkanDevice>())
{
	init();
}

void VulkanAllocator::init()
{
	createAllocator();
}

void VulkanAllocator::createAllocator()
{
	vma::AllocatorCreateInfo allocatorCreateInfo {
		.physicalDevice   = vulkanCore.getPhysicalDevice(),
		.device           = vulkanDevice.getDevice(),
		.instance         = vulkanCore.getInstance(),
		.vulkanApiVersion = vk::ApiVersion14
	};

	allocator = vma::createAllocatorUnique(allocatorCreateInfo);
}

vma::Allocator VulkanAllocator::getAllocator()
{
	return *allocator;
}

VulkanBuffer VulkanAllocator::allocate()
{
	VulkanBuffer buffer;

	vk::BufferCreateInfo bufferCreateInfo {
		//TODO
	};

	vma::AllocationCreateInfo allocationCreateInfo {
		//TODO
	};

	auto [bbuffer, allocation] = getAllocator().createBufferUnique(bufferCreateInfo, allocationCreateInfo, &buffer.info);

	buffer.buffer = std::move(bbuffer);
	buffer.allocation = std::move(allocation);

	return buffer;
}

void VulkanAllocator::flush(VulkanBuffer& buffer, vk::DeviceSize offset, vk::DeviceSize size)
{
	getAllocator().flushAllocation(buffer.getAllocation(), offset, size);
}

void VulkanAllocator::flush(std::span<VulkanBuffer> buffers, std::span<const vk::DeviceSize> offsets, std::span<const vk::DeviceSize> sizes)
{
	auto allocations = buffers |
		std::views::transform([](auto& x){return x.getAllocation();}) |
		std::ranges::to<std::vector>();

	getAllocator().flushAllocations(
		allocations,
		offsets,
		sizes
	);
}
