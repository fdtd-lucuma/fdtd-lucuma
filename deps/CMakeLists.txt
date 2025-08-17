# Una GUI para fdtd
# Copyright © 2025 Otreblan
#
# fdtd-vulkan is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# fdtd-vulkan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with fdtd-vulkan.  If not, see <http://www.gnu.org/licenses/>.


FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG        origin/docking
)

FetchContent_MakeAvailable(imgui)

add_library(imgui)

# imgui fails when unity builds are active
set_target_properties(imgui
	PROPERTIES
		UNITY_BUILD OFF
)

target_sources(imgui
	PRIVATE
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
		${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui
	PUBLIC
		${imgui_SOURCE_DIR}
)
