// Una GUI para fdtd
// Copyright © 2025 Otreblan
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

#include <cassert>

export module lucuma.services.backends:saver;

import lucuma.utils;
import lucuma.legacy_headers.mdspan;
import lucuma.legacy_headers.taskflow;

import :sequential_fdtd_data;
import :utils;

import std.compat;
import glm;

namespace lucuma::services::backends
{

struct SaverCreateInfo
{
	const std::filesystem::path& basePath;
};

template <class T>
class Saver
{
public:
	using data_t = FdtdData<T>;

	Saver(const SaverCreateInfo& createInfo):
		basePath(createInfo.basePath),
		datosCampoDir(basePath / "Datos_campo")
	{
	}

	void start(const data_t& data)
	{
		createBaseDir();
		writeInfo(data);
		writeMorfo(data);
	}

	void snapshot(const data_t& data)
	{
		// TODO: Move this to its own service
		static tf::Executor executor;
		tf::Taskflow taskflow;

		taskflow.name("File saver");

		const auto time = data.getTime();

		for(auto&& [name, mat]: data.zippedFields())
		{
			taskflow.emplace([=, this](){writeMatrix(name, time, mat);}).name(name);
		}

		executor.run(taskflow).wait();

		std::ofstream ofs("taskflow.dot");
		taskflow.dump(ofs);
	}

private:
	std::filesystem::path basePath;
	std::filesystem::path datosCampoDir;


	void createBaseDir()
	{
		if(!std::filesystem::exists(datosCampoDir))
			std::filesystem::create_directory(datosCampoDir);
	}

	template <typename... Args>
	void printAll(std::ostream& os, Args&&... args) {
		(std::println(os, "{}", std::forward<Args>(args)), ...);
	}

	template <typename F>
	void writeToFile(const std::filesystem::path& fileName, F f)
	{
		const auto morfoPath = basePath/fileName;
		auto ofs = std::ofstream(morfoPath);

		if(!ofs.is_open())
		{
			perror(morfoPath.c_str());
			return;
		}

		// Why is locale so slow?
		ofs.imbue(std::locale::classic());
		f(ofs);

	}

	void writeInfo(const data_t& data)
	{
		writeToFile("Info.txt", [&](std::ostream& os)
		{
			printAll(os,
				data.maxTime,
				data.size.x,
				data.size.y,
				data.size.z,
				data.maxTime
			);
		});

	}

	static void writeMorfoLine(std::ostream& os, data_t::cmdspan_3d_t mat)
	{
		std::println(os, "{} {} {}", mat.extent(0), mat.extent(1), mat.extent(2));
	}

	void writeMorfo(const data_t& data)
	{
		writeToFile("Morfo.txt", [&](std::ostream& os)
		{
			writeMorfoLine(os, data.Hx());
			writeMorfoLine(os, data.Hy());
			writeMorfoLine(os, data.Hz());

			writeMorfoLine(os, data.Ex());
			writeMorfoLine(os, data.Ey());
			writeMorfoLine(os, data.Ez());
		});

	}

	void writeMatrix(std::string_view name, unsigned int time, data_t::cmdspan_3d_t mat)
	{
		writeToFile(datosCampoDir/std::format("{}{}.txt", name, time), [&](std::ostream& os)
		{
			for(std::size_t i = 0; i < mat.extent(0); i++)
			{
				for(std::size_t j = 0; j < mat.extent(1); j++)
				{
					for(std::size_t k = 0; k < mat.extent(2); k++)
					{
						std::println(os, "{}", toPrintable(mat[i,j,k]));
					}
				}
			}
		});
	}

};

}
