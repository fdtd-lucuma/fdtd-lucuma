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

module fdtd.services;

import std;

VulkanDebug::VulkanDebug([[maybe_unused]] Injector& injector):
	vulkanContext(injector.inject<VulkanContext>())
{
}

std::vector<const char*> VulkanDebug::getRequiredLayers()
{
	std::vector<const char*> result;

	if(enableValidationLayers)
		result.append_range(validationLayers);

	checkLayers(result);

	return result;
}

bool noAnyLayerInProperties(
	std::span<const char* const> requiredLayers,
	std::span<const vk::LayerProperties> layerProperties
)
{
	return std::ranges::any_of(requiredLayers,
		[&](const auto& requiredLayer)
		{
			return std::ranges::none_of(layerProperties,
				[&](const auto& layerProperty)
				{
					return strcmp(layerProperty.layerName, requiredLayer) == 0;
				}
			);
		}
	);
}

void VulkanDebug::checkLayers(std::span<const char* const> requiredLayers)
{
	const auto layerProperties = vulkanContext.getContext().enumerateInstanceLayerProperties();

	if(noAnyLayerInProperties(requiredLayers, layerProperties))
	{
		std::cerr << "Required layers:\n";

		for(const auto& layer: requiredLayers)
			std::cerr << '\t' << layer << '\n';

		throw std::runtime_error("One or more required layers are not supported");
	}

}
