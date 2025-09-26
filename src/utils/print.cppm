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

export module lucuma.utils:print;

import std.compat;

namespace lucuma::utils
{

export template <typename... Args>
void printAll(std::ostream& os, Args&&... args) {
	(std::println(os, "{}", std::forward<Args>(args)), ...);
}

export template <typename F>
void writeToFile(const std::filesystem::path& path, F&& f)
{
	auto ofs = std::ofstream(path);

	if(!ofs.is_open())
	{
		perror(path.c_str());
		return;
	}

	// Why is locale so slow?
	ofs.imbue(std::locale::classic());
	f(ofs);

}


}
