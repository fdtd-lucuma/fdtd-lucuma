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

export module lucuma.services.vulkan:swapchain;

import vulkan_hpp;
import std;

import lucuma.utils;
import lucuma.services.basic;
import lucuma.services.window;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::services;

class Device;
class Core;

struct SwapchainDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> availableFormats;
	std::vector<vk::PresentModeKHR> availablePresentModes;
};


export class Swapchain
{
public:
	Swapchain(Injector& injector);

	vk::Format   getFormat() const;
	vk::Extent2D getExtent() const;

	vk::Viewport getCurrentViewport() const;
	vk::Rect2D   getCurrentScissor()  const;

private:
	Core&            core;
	Device&          device;
	basic::Settings& settings;
	window::Glfw&    glfw;

	vk::Format   format;
	vk::Extent2D extent;

	vk::raii::SwapchainKHR           swapchain = nullptr;
	std::vector<vk::Image>           swapchainImages;
	std::vector<vk::raii::ImageView> swapchainImageViews;

	void init();
	void createSwapchain();
	void createSwapchainImageViews();

	SwapchainDetails getDetails() const;

	vk::SurfaceFormatKHR selectDefaultSurfaceFormat(const SwapchainDetails& details) const;
	vk::PresentModeKHR selectDefaultPresentMode(const SwapchainDetails& details) const;
	vk::Extent2D selectDefaultExtent(const SwapchainDetails& details) const;

};

}
