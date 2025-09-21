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

export module lucuma.services.basic:settings;

import :settings_enums;

import lucuma.utils;
import std;
import glm;

namespace lucuma::services::basic
{

using namespace lucuma::utils;

class ArgumentParser;

export class Settings
{
public:
	Settings(Injector& injector);

	bool isHeadless() const;

	static constexpr std::size_t defaultSizeX = 128;
	static constexpr std::size_t defaultSizeY = 128;
	static constexpr std::size_t defaultSizeZ = 128;

	static constexpr unsigned int defaultTime = 300;

	static constexpr Backend defaultBackend = Backend::sequential;

	std::size_t sizeX() const;
	std::size_t sizeY() const;
	std::size_t sizeZ() const;

	unsigned int time() const;

	glm::vec<3, std::size_t> size() const;

	Backend backend() const;

private:
	ArgumentParser& argumentParser;

};

}
