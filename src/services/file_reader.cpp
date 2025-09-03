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

#include <cassert>
#include "../macros.hpp"

#if (HAS_MMAP==1)
#    include <sys/mman.h>
#    include <sys/stat.h>
#    include <unistd.h>
#    include <cstring>
#    include <cerrno>
#    include <fcntl.h>
#    include <cstdio>
#endif

module fdtd.services;

import std;

void cleanFd(int* fd)
{
	assert(fd != nullptr);

	close(*fd);
}

void FileBuffer::readIntoVector(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);

	if(!file.is_open())
		throwFile(path);

	copyBuffer = std::vector<char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	bufferType = BufferType::COPY;
}

bool FileBuffer::readIntoMmap(const std::filesystem::path& path)
{
#if (HAS_MMAP==0)
	return false;
#else
	[[gnu::cleanup(cleanFd)]]
	int fd = open(path.c_str(), O_RDONLY);

	if(fd == -1)
		throwFile(path);

	if(fstat(fd, &mmapData.sb) == -1)
		throwFile(path);

	if(mmapData.sb.st_size == 0)
		return false;

	if (!(S_ISREG(mmapData.sb.st_mode) || S_ISBLK(mmapData.sb.st_mode)))
		return false;

	mmapData.buffer = (char*)mmap(nullptr, mmapData.sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(mmapData.buffer == MAP_FAILED)
	{
		perror(path.c_str());
		return false;
	}

	bufferType = BufferType::MMAP;
	return true;
#endif
}

FileBuffer::FileBuffer(FileBuffer&& other):
	bufferType(std::exchange(other.bufferType, {})),
	copyBuffer(std::exchange(other.copyBuffer, {}))
#if (HAS_MMAP==1)
	,mmapData(std::exchange(other.mmapData, {}))
#endif
	{}

FileBuffer::FileBuffer(const std::filesystem::path& path)
{
	if(!readIntoMmap(path))
		readIntoVector(path);
}

FileBuffer::~FileBuffer()
{
#if (HAS_MMAP==1)
	if(bufferType == BufferType::MMAP)
	{
		if(munmap(mmapData.buffer, mmapData.sb.st_size) < 0)
			perror("munmap");
	}
#endif
}

template<>
std::span<const char> FileBuffer::getBuffer<char>() const
{
	switch(bufferType)
	{
		case BufferType::COPY:
			return copyBuffer;
#if (HAS_MMAP==1)
		case BufferType::MMAP:
			return std::span<char const>(mmapData.buffer, mmapData.sb.st_size);
#endif
		default:
			return std::span<char const>{};
	}
}

FileReader::FileReader([[maybe_unused]]Injector& injector)
{ }


FileBuffer FileReader::read(const std::filesystem::path& path)
{
	return FileBuffer(path);
}
