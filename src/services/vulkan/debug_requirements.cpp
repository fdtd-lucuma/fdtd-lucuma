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

module lucuma.services.vulkan;

namespace lucuma::services::vulkan
{

DebugRequirements::DebugRequirements([[maybe_unused]] Injector& injector)
{
}

std::vector<const char*> DebugRequirements::getRequiredLayers()
{
	std::vector<const char*> result;

	if constexpr(enableValidationLayers)
		result.append_range(validationLayers);

	return result;
}

std::vector<const char*> DebugRequirements::getRequiredExtensions()
{
	std::vector<const char*> result;

	if constexpr(enableValidationLayers)
		result.emplace_back(vk::EXTDebugUtilsExtensionName);

	return result;
}

}
