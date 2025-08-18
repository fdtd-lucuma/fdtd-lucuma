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

if(NOT USE_SYSTEM_LIBS)
	FetchContent_Declare(
		EnTT
		GIT_REPOSITORY https://github.com/skypjack/entt
		GIT_TAG        v3.15.0
		GIT_SHALLOW    ON
		GIT_PROGRESS   ON
		OVERRIDE_FIND_PACKAGE
	)

	FetchContent_MakeAvailable(EnTT)
endif()

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui
	GIT_TAG        origin/docking
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	VulkanMemoryAllocator-Hpp
	GIT_REPOSITORY         https://github.com/YaaZ/VulkanMemoryAllocator-Hpp
	GIT_TAG                v3.2.1
	GIT_SHALLOW            ON
	GIT_PROGRESS           ON
	GIT_SUBMODULES         ""
	GIT_SUBMODULES_RECURSE OFF
	OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(imgui VulkanMemoryAllocator-Hpp)

add_library(imgui)

# C++ version
set_target_properties(imgui
	PROPERTIES
		CXX_STANDARD 23
)

# imgui fails when unity builds are active
set_target_properties(imgui
	PROPERTIES
		UNITY_BUILD OFF
)

target_compile_definitions(imgui
	PRIVATE
		$<$<CONFIG:DEBUG>:VK_DEBUG>
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
		GLFW_INCLUDE_VULKAN
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

# Packages
find_package(bash-completion REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)
find_package(VulkanMemoryAllocator-Hpp REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

# Shared libraries
pkg_check_modules(libraries
	REQUIRED IMPORTED_TARGET GLOBAL
		glfw3
)

# set up Vulkan C++ module as a library
add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${Vulkan_INCLUDE_DIR}
	FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
set_target_properties(VulkanHppModule
	PROPERTIES
		CXX_STANDARD 23
)

target_compile_definitions(VulkanHppModule
	PRIVATE
		$<$<CONFIG:DEBUG>:VK_DEBUG>
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::libraries
		imgui
		EnTT::EnTT
		VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
		VulkanHppModule

	INTERFACE
		GPUOpen::VulkanMemoryAllocator
		glm::glm-header-only
)

