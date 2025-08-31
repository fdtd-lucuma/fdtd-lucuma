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

FetchContent_Declare(
	vkfw
	GIT_REPOSITORY https://github.com/Cvelth/vkfw
	GIT_TAG        origin/main
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
	OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(imgui VulkanMemoryAllocator-Hpp vkfw)

# Packages
find_package(VulkanMemoryAllocator REQUIRED)
find_package(VulkanMemoryAllocator-Hpp REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vkfw REQUIRED)

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
		CXX_STANDARD 26
)

target_compile_definitions(VulkanHppModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
		VULKAN_HPP_ENABLE_STD_MODULE
		VULKAN_HPP_STD_MODULE=std
)

target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)

# set up vkfw C++ module as a library
add_library(VkfwModule)
target_sources(VkfwModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${vkfw_SOURCE_DIR}
	FILES ${vkfw_SOURCE_DIR}/include/vkfw/vkfw.cppm
)
set_target_properties(VkfwModule
	PROPERTIES
		CXX_STANDARD 26
)

target_compile_definitions(VkfwModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		# TODO: Find out how to dedup vulkan module target
		#VKFW_NO_INCLUDE_VULKAN
		VKFW_NO_INCLUDE_VULKAN_HPP
		VKFW_ENABLE_VULKAN_HPP_MODULE
		#VKFW_ENABLE_STD_MODULE
		VKFW_NO_STRUCT_CONSTRUCTORS
)

target_link_libraries(VkfwModule
	PUBLIC
		VulkanHppModule
		vkfw
)

# TODO: Find what to do when the library doesn't have an alias
target_include_directories(VkfwModule
	PRIVATE
		${vkfw_SOURCE_DIR}/include
)

add_library(imgui)

# C++ version
set_target_properties(imgui
	PROPERTIES
		CXX_STANDARD 26
)

# imgui fails when unity builds are active
set_target_properties(imgui
	PROPERTIES
		UNITY_BUILD OFF
)

target_compile_definitions(imgui
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
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

target_link_libraries(imgui
	PUBLIC
		VkfwModule
)

# set up glm C++ module as a library
add_library(GlmModule)
target_sources(GlmModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS /usr/include # How can I get the glm include dir?
	FILES /usr/include/glm/glm.cppm
)
set_target_properties(GlmModule
	PROPERTIES
		CXX_STANDARD 26
)

target_compile_definitions(GlmModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		GLM_FORCE_DEPTH_ZERO_TO_ONE
		GLM_FORCE_RADIANS
)

target_link_libraries(GlmModule PUBLIC glm::glm-header-only)

# set up vma hpp C++ module as a library
add_library(VulkanMemoryAllocator-HppModule)
target_sources(VulkanMemoryAllocator-HppModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${VulkanMemoryAllocator-Hpp_SOURCE_DIR}
	FILES ${VulkanMemoryAllocator-Hpp_SOURCE_DIR}/src/vk_mem_alloc.cppm
)
set_target_properties(VulkanMemoryAllocator-HppModule
	PROPERTIES
		CXX_STANDARD 26
)

target_compile_definitions(VulkanMemoryAllocator-HppModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		VMA_DYNAMIC_VULKAN_FUNCTIONS=1
		VMA_STATIC_VULKAN_FUNCTIONS=0
		VMA_ENABLE_VULKAN_HPP_MODULE
)

target_link_libraries(VulkanMemoryAllocator-HppModule
	PUBLIC
		VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
		VulkanHppModule

	INTERFACE
		GPUOpen::VulkanMemoryAllocator
)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::libraries
		imgui
		EnTT::EnTT
		VulkanMemoryAllocator-HppModule
		GlmModule
		VkfwModule
)

