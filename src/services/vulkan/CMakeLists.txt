# Una GUI para fdtd
# Copyright © 2025-2026 Otreblan
#
# fdtd-lucuma is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# fdtd-lucuma is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with fdtd-lucuma.  If not, see <http://www.gnu.org/licenses/>.

sources(
	PRIVATE
		all.cpp
		allocator.cpp
		compute.cpp
		context.cpp
		core.cpp
		debug.cpp
		debug_requirements.cpp
		device.cpp
		graphics.cpp
		imgui.cpp
		instantiations.cpp
		shader_loader.cpp
		swapchain.cpp
		triangle_demo.cpp
		utils.cpp
	PRIVATE
		FILE_SET fdtd
		TYPE CXX_MODULES
		FILES
			all.cppm
			allocator.cppm
			compute.cppm
			context.cppm
			core.cppm
			debug.cppm
			debug_requirements.cppm
			device.cppm
			graphics.cppm
			imgui.cppm
			shader_loader.cppm
			swapchain.cppm
			triangle_demo.cppm
			utils.cppm
			vulkan.cppm
)
