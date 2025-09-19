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

module fdtd.services.basic;

import std;
import glm;

namespace fdtd::services::basic
{

Settings::Settings([[maybe_unused]]Injector& injector):
	argumentParser(injector.inject<ArgumentParser>())
{ }

bool Settings::isHeadless() const
{
	return argumentParser.isHeadless();
}

std::size_t Settings::sizeX() const
{
	return argumentParser.sizeX().value_or(defaultSizeX);
}

std::size_t Settings::sizeY() const
{
	return argumentParser.sizeY().value_or(defaultSizeY);
}

std::size_t Settings::sizeZ() const
{
	return argumentParser.sizeZ().value_or(defaultSizeZ);
}

glm::vec3 Settings::size() const
{
	return glm::vec3{sizeX(), sizeY(), sizeZ()};
}


}
