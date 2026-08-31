// Una GUI para fdtd
// Copyright © 2025-2026 Otreblan
//
// fdtd-lucuma is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fdtd-lucuma is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fdtd-lucuma.  If not, see <http://www.gnu.org/licenses/>.

module;

#include "../../macros.hpp"
#include <cassert>

#if (HAS_MMAP==1)
#    include <sys/mman.h>
#    include <sys/stat.h>
#endif

export module lucuma.services.basic:file_reader;

import lucuma.utils;
import lucuma.legacy_headers.fast_float;

import std;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

template <typename T>
void assertAligned([[maybe_unused]] const void* ptr) {
	assert(reinterpret_cast<std::uintptr_t>(ptr) % alignof(T) == 0 && "Pointer is not properly aligned for T");
}


export class FileBuffer
{
public:
	FileBuffer(FileBuffer const&) = delete;
	FileBuffer(FileBuffer&& other);

	FileBuffer& operator=(FileBuffer const&) = delete;
	FileBuffer& operator=(FileBuffer&&)      = default;

	FileBuffer(const std::filesystem::path& path);
	~FileBuffer();

	template<typename T = char>
	std::span<const T> getBuffer() const
	{
		auto span = getBuffer();

		assertAligned<T>(span.data());
		assert(span.size() % sizeof(T) == 0);

		return {(const T*)span.data(), span.size_bytes() / sizeof(T)};
	}

	template<>
	std::span<const char> getBuffer() const;

	template <typename T>
	operator std::span<const T>() const
	{
		return getBuffer<T>();
	}

private:
	enum class BufferType
	{
		COPY,
		MMAP,
		MOVED_FROM
	} bufferType = BufferType::MOVED_FROM;

	std::vector<char> copyBuffer;

	void readIntoVector(const std::filesystem::path& path);

#if (HAS_MMAP==1)
	struct
	{
		char* buffer = nullptr;
		struct ::stat sb;
	} mmapData;
#endif

	bool readIntoMmap(const std::filesystem::path& path);

};

export
class FileToFloat
{
public:
	template <typename T>
	struct Iterator
	{
		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = T;
		using pointer           = T*;
		using reference         = T&;

		// TODO: Iterate readOne() until it returns false
	};

	FileToFloat(FileBuffer&& buffer):
		buffer(std::move(buffer)),
		currentPtr(this->buffer.getBuffer().data())
	{}

	template <typename T>
	requires std::is_floating_point_v<T>
	bool readOne(T& result)
	{
		assert(currentPtr != nullptr);
		const auto span = buffer.getBuffer();
		auto answer = fast_float::from_chars<T, char>(currentPtr, span.data() + span.size(), result);

		currentPtr = answer.ptr+1;

		return (bool)answer;
	}

	template <typename T>
	bool readOne(T& result)
	{
		float fresult;
		auto answer = readOne<float>(fresult);
		result = fresult;

		return answer;
	}

private:
	FileBuffer  buffer;
	const char* currentPtr;
};

export class FileReader
{
public:
	FileReader(Injector& injector);

	FileBuffer read(const std::filesystem::path& path);

	FileToFloat readIntoFloats(const std::filesystem::path& path)
	{
		return FileToFloat(read(path));
	}
private:
};

}
