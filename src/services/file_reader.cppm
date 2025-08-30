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

#include "../macros.hpp"

#if (HAS_MMAP==1)
#    include <sys/mman.h>
#    include <sys/stat.h>
#endif

export module fdtd.services:file_reader;

export import fdtd.utils;

import std;

export class FileBuffer
{
public:
	FileBuffer(FileBuffer const&) = delete;
	FileBuffer(FileBuffer&& other);

	FileBuffer& operator=(FileBuffer const&) = delete;
	FileBuffer& operator=(FileBuffer&&)      = default;

	FileBuffer(const std::filesystem::path& path);
	~FileBuffer();

	std::span<char const> getBuffer() const;

	operator std::span<char const>() const;

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

export class FileReader
{
public:
	FileReader(Injector& injector);

	FileBuffer read(const std::filesystem::path& path);
private:
};
