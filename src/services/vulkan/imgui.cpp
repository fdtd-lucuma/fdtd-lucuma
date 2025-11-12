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

import lucuma.services.window;
import lucuma.services.basic;
import lucuma.legacy_headers.entt;
import lucuma.events;

import imgui;
import imgui_impl_vulkan;
import imgui_impl_glfw;
import vulkan_hpp;

namespace lucuma::services::vulkan
{

using namespace lucuma::services;

Imgui::Imgui([[maybe_unused]] Injector& injector):
	dispatcher(injector.inject<entt::dispatcher>()),
	registry(injector.inject<entt::registry>()),
	core(injector.inject<Core>()),
	device(injector.inject<Device>()),
	swapchain(injector.inject<Swapchain>()),
	graphics(injector.inject<Graphics>()),
	glfw(injector.inject<window::Glfw>())

{
	init();
}

void Imgui::init()
{
	initImgui();

	dispatcher.sink<events::GuiDraw>().connect<&Imgui::onDraw>(*this);
	dispatcher.sink<events::FrameStart>().connect<&Imgui::onFrameStart>(*this);
	dispatcher.sink<events::FrameEnd>().connect<&Imgui::onFrameEnd>(*this);
	dispatcher.sink<events::Update>().connect<&Imgui::onUpdate>(*this);
}

void Imgui::initImgui()
{
	ImGui::CheckVersion();
	ImGui::CreateContext();

	auto& io = ImGui::GetIO();

	io.ConfigFlags |=
		ImGuiConfigFlags_NavEnableKeyboard |
		ImGuiConfigFlags_NavEnableGamepad |
		ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(glfw.getWindow(), true);

	const auto swapchainFormat = swapchain.getFormat();

	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo;
	pipelineRenderingCreateInfo.setColorAttachmentFormats(swapchainFormat);

	ImGui_ImplVulkan_InitInfo initInfo {
		.ApiVersion         = core.getApiVersion(),
		.Instance           = *core.getInstance(),
		.PhysicalDevice     = *core.getPhysicalDevice(),
		.Device             = *device.getDevice(),
		.QueueFamily        = device.getGraphicsInfo()->index,
		.Queue              = *graphics.getGraphicsQueue(),
		.DescriptorPool     = {},
		.DescriptorPoolSize = 128,
		.MinImageCount      = 2,
		.ImageCount         = 2,
		.PipelineCache      = {},
		.PipelineInfoMain   = {
			.RenderPass                  = {},
			.Subpass                     = {},
			.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo,
			.SwapChainImageUsage         = {},
		},
		.PipelineInfoForViewports   = {},
		.UseDynamicRendering        = true,
		.Allocator                  = {},
		.CheckVkResultFn            = {},
		.MinAllocationSize          = 1024*1024,
		.CustomShaderVertCreateInfo = {},
		.CustomShaderFragCreateInfo = {},
	};

	ImGui_ImplVulkan_Init(&initInfo);
}

void Imgui::onDraw(const events::GuiDraw& event)
{
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), event.commandBuffer);
}

void Imgui::onFrameStart(const events::FrameStart&)
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();

	ImGui::NewFrame();
}

void Imgui::onFrameEnd(const events::FrameEnd&)
{
	ImGui::Render();
}
void Imgui::onUpdate(const events::Update&)
{
	ImGui::ShowDemoWindow();
}

Imgui::~Imgui()
{
	dispatcher.sink<events::GuiDraw>().disconnect<&Imgui::onDraw>(*this);
	dispatcher.sink<events::FrameStart>().disconnect<&Imgui::onFrameStart>(*this);
	dispatcher.sink<events::FrameEnd>().disconnect<&Imgui::onFrameEnd>(*this);
	dispatcher.sink<events::Update>().disconnect<&Imgui::onUpdate>(*this);

	device.waitIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

}
