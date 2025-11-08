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

export module lucuma.services.vulkan:graphics;

import vulkan_hpp;
import std;

import lucuma.utils;
import lucuma.utils.vulkan;
import lucuma.services.window;

namespace lucuma::services::vulkan
{

using namespace lucuma::utils;
using namespace lucuma::utils::vulkan;
using namespace lucuma::services;

class Buffer;
class Device;
class ShaderLoader;
class Swapchain;
struct QueueFamilyInfo;

struct TransitionImageLayoutInfo {
	std::uint32_t           imageIndex    = {};
	vk::ImageLayout         oldLayout     = {};
	vk::ImageLayout         newLayout     = {};
	vk::AccessFlags2        srcAccessMask = {};
	vk::AccessFlags2        dstAccessMask = {};
	vk::PipelineStageFlags2 srcStageMask  = {};
	vk::PipelineStageFlags2 dstStageMask  = {};
};

export class Graphics
{
public:
	Graphics(Injector& injector);

	vk::raii::Queue&       getGraphicsQueue();
	vk::raii::CommandPool& getGraphicsCommandPool();

	vk::raii::Queue&       getPresentQueue();
	vk::raii::CommandPool& getPresentCommandPool();

	vk::Result acquireNextImage();

	void draw();
	void waitFence();
	vk::Result present();

	~Graphics();

private:
	constexpr static int MAX_FRAMES_IN_FLIGHT = 2;

	Device&       device;
	ShaderLoader& shaderLoader;
	Swapchain&    swapchain;
	window::Glfw& glfw;

	std::uint32_t currentImageIndex;

	std::vector<vk::raii::Queue> graphicsQueues;
	vk::raii::CommandPool		graphicsCommandPool = nullptr;

	std::vector<vk::raii::Queue> presentQueues;
	vk::raii::CommandPool		presentCommandPool = nullptr;

	void init();

	void createQueues();
	void createCommandPools();

	std::vector<vk::raii::Queue> createQueuesCommon(const QueueFamilyInfo& info);
	vk::raii::CommandPool createCommandPool(const QueueFamilyInfo& info);

	void createGraphicsPipeline(); //TODO: Move this into its own object
	void createCommandBuffers(); //TODO: Move this into its own object
	void createSyncObjects();

	void recordCommandBuffer(std::uint32_t imageIndex);
	void transition_image_layout(const TransitionImageLayoutInfo& input);

	void transitionImageAny2Optimal(std::uint32_t imageIndex);
	void transitionImageOptimal2PresentSrc(std::uint32_t imageIndex);

	std::uint32_t currentFrame = 0;
	bool framebufferResized = false;

	void advanceFrame();

	// Triangle data. TODO: Move this into its own object
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::Pipeline       pipeline       = nullptr;

	// TODO: Move this even more outside
	std::vector<vk::raii::CommandBuffer> commandBuffers;

	std::vector<vk::raii::Fence>     inFlightFences;

	vk::raii::CommandBuffer& getCurrentCommandBuffer();
	vk::raii::Fence&         getCurrentInFlightFence();

};

}
