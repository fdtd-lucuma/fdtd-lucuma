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

export module fdtd.services.vulkan:allocator;

import std;
import vk_mem_alloc_hpp;
import vulkan_hpp;

import fdtd.utils;

namespace fdtd::services::vulkan
{

using namespace fdtd::utils;

export class Buffer
{
public:
	Buffer() = default;

	Buffer(Buffer const&) = delete;
	Buffer(Buffer&& other);

	Buffer& operator=(Buffer const&) = delete;
	Buffer& operator=(Buffer&&)      = default;

	vk::Buffer          getBuffer() const;
	vma::AllocationInfo getInfo() const;
	vma::Allocation     getAllocation() const;

	// Like memcpy(3) but dest is the internal buffer
	void* memcpy(const void* src, std::size_t n);

	template<typename T>
	void setData(std::span<const T> data)
	{
		memcpy(data.data(), data.size_bytes());
	}

	template<typename T>
	std::span<T> getData() const
	{
		const auto info = getInfo();
		return {(T*)info.pMappedData, info.size/sizeof(T)};
	}

private:

	vma::UniqueBuffer     buffer;
	vma::AllocationInfo   info;
	vma::UniqueAllocation allocation;

	friend class Allocator;
};

class Core;
class Device;

export class Allocator
{
public:
	Allocator(Injector& injector);

	vma::Allocator& getAllocator();

	Buffer allocate(vk::DeviceSize size, vk::BufferUsageFlags usage, vma::AllocationCreateFlags flags);

	void flush(Buffer& buffer, vk::DeviceSize offset = 0, vk::DeviceSize size = vk::WholeSize);
	vk::Result flush(
		std::span<const std::reference_wrapper<const Buffer>> buffers,
		std::span<const vk::DeviceSize> offsets = {},
		std::span<const vk::DeviceSize> sizes = {}
	);

private:
	Core&   core;
	Device& device;

	vma::UniqueAllocator allocator;

	void init();

	void createAllocator();

};

}
