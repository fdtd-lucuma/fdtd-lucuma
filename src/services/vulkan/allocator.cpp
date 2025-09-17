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
import std.compat;

namespace fdtd::services::vulkan
{

Buffer::Buffer(Buffer&& other):
	buffer(std::exchange(other.buffer, {})),
	info(std::exchange(other.info, {})),
	allocation(std::exchange(other.allocation, {}))
{}

vk::Buffer Buffer::getBuffer() const
{
	return *buffer;
}

vma::AllocationInfo Buffer::getInfo() const
{
	return info;
}

vma::Allocation Buffer::getAllocation() const
{
	return *allocation;
}

void* Buffer::memcpy(const void* src, std::size_t n)
{
	return ::memcpy(getInfo().pMappedData, src, n);
}

Allocator::Allocator(Injector& injector):
	core(injector.inject<Core>()),
	device(injector.inject<Device>())
{
	init();
}

void Allocator::init()
{
	createAllocator();
}

void Allocator::createAllocator()
{
	vma::AllocatorCreateInfo allocatorCreateInfo {
		.physicalDevice   = core.getPhysicalDevice(),
		.device           = device.getDevice(),
		.instance         = core.getInstance(),
		.vulkanApiVersion = vk::ApiVersion14
	};

	allocator = vma::createAllocatorUnique(allocatorCreateInfo);
}

vma::Allocator& Allocator::getAllocator()
{
	return allocator.get();
}

Buffer Allocator::allocate(vk::DeviceSize size, vk::BufferUsageFlags usage, vma::AllocationCreateFlags flags)
{
	Buffer buffer;

	vk::BufferCreateInfo bufferCreateInfo {
		.size  = size,
		.usage = usage,
	};

	vma::AllocationCreateInfo allocationCreateInfo {
		.flags = flags,
		.usage = vma::MemoryUsage::eAuto,
	};

	std::tie(buffer.buffer, buffer.allocation) = getAllocator().createBufferUnique(bufferCreateInfo, allocationCreateInfo, &buffer.info);

	return buffer;
}

void Allocator::flush(Buffer& buffer, vk::DeviceSize offset, vk::DeviceSize size)
{
	getAllocator().flushAllocation(buffer.getAllocation(), offset, size);
}

vk::Result Allocator::flush(
	std::span<const std::reference_wrapper<const Buffer>> buffers,
	std::span<const vk::DeviceSize> offsets,
	std::span<const vk::DeviceSize> sizes
)
{
	auto allocations = buffers |
		std::views::transform([](const auto& x) {return x.get().getAllocation();}) |
		std::ranges::to<std::vector>();

	return getAllocator().flushAllocations(
		allocations.size(), allocations.data(),
		offsets.empty() ? nullptr : offsets.data(),
		sizes.empty() ? nullptr : sizes.data()
	);
}

}
