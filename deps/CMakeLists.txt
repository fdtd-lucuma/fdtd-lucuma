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

if(NOT USE_SYSTEM_LIBS)
	FetchContent_Declare(
		EnTT
		GIT_REPOSITORY https://github.com/skypjack/entt.git
		GIT_TAG        v3.16.0
		GIT_SHALLOW    ON
		GIT_PROGRESS   ON
		OVERRIDE_FIND_PACKAGE
	)

	FetchContent_MakeAvailable(EnTT)
endif()

if(BUILD_FOR_TERMUX)

	set(TF_BUILD_TESTS    FALSE CACHE BOOL "" FORCE)
	set(TF_BUILD_EXAMPLES FALSE CACHE BOOL "" FORCE)

	FetchContent_Declare(
		taskflow
		GIT_REPOSITORY https://github.com/taskflow/taskflow
		GIT_TAG        v4.0.0
		GIT_SHALLOW    ON
		GIT_PROGRESS   ON
		GIT_SUBMODULES         ""
		GIT_SUBMODULES_RECURSE OFF
		OVERRIDE_FIND_PACKAGE
	)

	FetchContent_Declare(
		fastfloat
		GIT_REPOSITORY https://github.com/fastfloat/fast_float
		GIT_TAG        v8.2.5
		GIT_SHALLOW    ON
		GIT_PROGRESS   ON
		GIT_SUBMODULES         ""
		GIT_SUBMODULES_RECURSE OFF
		OVERRIDE_FIND_PACKAGE
	)

	FetchContent_MakeAvailable(taskflow fastfloat)

endif()

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	#GIT_TAG        origin/docking
	GIT_TAG        60d7fb207eeb46d6363dd4bde10b35991bae0ce7
	#GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	imgui-module
	#GIT_REPOSITORY         https://github.com/stripe2933/imgui-module.git
	GIT_REPOSITORY         https://github.com/otreblan/imgui-module.git
	GIT_TAG                origin/docking
	GIT_SHALLOW            ON
	GIT_PROGRESS           ON
	GIT_SUBMODULES         ""
	GIT_SUBMODULES_RECURSE OFF
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
	VulkanMemoryAllocator-Hpp
	GIT_REPOSITORY         https://github.com/YaaZ/VulkanMemoryAllocator-Hpp.git
	GIT_TAG                origin/master
	GIT_SHALLOW            ON
	GIT_PROGRESS           ON
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
	magic_enum
	GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
	GIT_TAG        v0.9.7
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
	mdspan
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY https://github.com/kokkos/mdspan.git
	GIT_TAG        stable
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	imgui-graphnode
	GIT_REPOSITORY https://github.com/otreblan/imgui-graphnode
	GIT_TAG        master
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	implot3d
	GIT_REPOSITORY https://github.com/brenocq/implot3d
	#GIT_TAG        main
	GIT_TAG        3d7fb60faf77552705b70c23600ff49f02ac5f93
	#GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	implot
	GIT_REPOSITORY https://github.com/epezent/implot
	#GIT_TAG        master
	GIT_TAG        81b8b1951392767cf458508385fa025fd087a252
	#GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

set(NFD_PORTAL ON CACHE BOOL "Use xdg-desktop-portal instead of GTK" FORCE)
FetchContent_Declare(
	nativefiledialog-extended
	GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended
	GIT_TAG        master
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
	OVERRIDE_FIND_PACKAGE
)

set(TRACY_DEBUGINFOD ON CACHE BOOL "Enable debuginfod support" FORCE)
set(TRACY_DELAYED_INIT ON CACHE BOOL "Enable delayed initialization of the library (init on first call)" FORCE)
set(TRACY_MANUAL_LIFETIME ON CACHE BOOL "Enable the manual lifetime management of the profile" FORCE)

FetchContent_Declare(
	font-awesome
	GIT_REPOSITORY https://github.com/FortAwesome/Font-Awesome
	GIT_TAG        7.x
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

FetchContent_Declare(
	icon-fonts-cpp-headers
	GIT_REPOSITORY https://github.com/juliettef/IconFontCppHeaders
	GIT_TAG        main
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
)

pkg_check_modules(debuginfod libdebuginfod)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT debuginfod_FOUND)
	set(TRACY_DEBUGINFOD OFF)
endif()

FetchContent_Declare(
	tracy
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY https://github.com/wolfpld/tracy.git
	GIT_TAG        v0.13.1
	#GIT_TAG        master
	GIT_SHALLOW    ON
	GIT_PROGRESS   ON
	OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(
	imgui
	imgui-module
	VulkanMemoryAllocator-Hpp
	magic_enum
	mdspan
	imgui-graphnode
	implot3d
	implot
	nativefiledialog-extended
	tracy
	font-awesome
	icon-fonts-cpp-headers
)

# Packages
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator-Hpp REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(slang REQUIRED) #TODO: Ubuntu https://github.com/shader-slang/slang/issues/8334
find_package(magic_enum REQUIRED)
find_package(XdgUtils COMPONENTS BaseDir)
find_package(Taskflow REQUIRED)
find_package(Threads REQUIRED)
find_package(nativefiledialog-extended REQUIRED)
find_package(FastFloat REQUIRED)
find_package(tracy REQUIRED)

set(FONT_AWESOME_SOURCE_DIR ${font-awesome_SOURCE_DIR} PARENT_SCOPE)

# For some readon mdspan install even if it's not at top level
set_target_properties(mdspan
	PROPERTIES
		EXCLUDE_FROM_ALL TRUE
)

# Shared libraries
pkg_check_modules(libraries
	REQUIRED IMPORTED_TARGET GLOBAL
		fmt
		freetype2
		libgvc
		sdl3
		simdjson
		x11
)

# set up Vulkan C++ module as a library
add_library(VulkanHppModule)
target_sources(VulkanHppModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${Vulkan_INCLUDE_DIR}
	FILES
		${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
		${Vulkan_INCLUDE_DIR}/vulkan/vulkan_video.cppm
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

target_sources(imgui
	PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${imgui-module_SOURCE_DIR}/generated
	FILES
		${imgui-module_SOURCE_DIR}/generated/imgui.cppm
		${imgui-module_SOURCE_DIR}/generated/backends/imgui_impl_sdl3.cppm
		${imgui-module_SOURCE_DIR}/generated/backends/imgui_impl_vulkan.cppm

	PRIVATE
		${imgui_SOURCE_DIR}/imgui.cpp
		${imgui_SOURCE_DIR}/imgui_demo.cpp
		${imgui_SOURCE_DIR}/imgui_draw.cpp
		${imgui_SOURCE_DIR}/imgui_tables.cpp
		${imgui_SOURCE_DIR}/imgui_widgets.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
		${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
		${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
		${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
)

target_include_directories(imgui
	PUBLIC
		${imgui_SOURCE_DIR}
		${imgui_SOURCE_DIR}/backends
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(imgui
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		IMGUI_DISABLE_OBSOLETE_FUNCTIONS
		IMGUI_ENABLE_FREETYPE
		IMGUI_USER_CONFIG="imconfig.hpp"
)

target_link_libraries(imgui
	PRIVATE
		PkgConfig::libraries
)

add_library(imgui-graphnode)

# C++ version
set_target_properties(imgui-graphnode
	PROPERTIES
		CXX_STANDARD 26
)

target_sources(imgui-graphnode
	PRIVATE
		${imgui-graphnode_SOURCE_DIR}/imgui_graphnode.cpp
		${imgui-graphnode_SOURCE_DIR}/imgui_graphnode_internal.cpp
)

target_include_directories(imgui-graphnode
	PUBLIC
		${imgui-graphnode_SOURCE_DIR}
)

target_compile_definitions(imgui-graphnode
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

target_link_libraries(imgui-graphnode
	PUBLIC
		imgui
	PRIVATE
		PkgConfig::libraries
)

add_library(implot3d)

# C++ version
set_target_properties(implot3d
	PROPERTIES
		CXX_STANDARD 26
)

target_sources(implot3d
	PRIVATE
		${implot3d_SOURCE_DIR}/implot3d.cpp
		${implot3d_SOURCE_DIR}/implot3d_demo.cpp
		${implot3d_SOURCE_DIR}/implot3d_items.cpp
		${implot3d_SOURCE_DIR}/implot3d_meshes.cpp
)

target_include_directories(implot3d
	PUBLIC
		${implot3d_SOURCE_DIR}
)

target_compile_definitions(implot3d
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

target_link_libraries(implot3d
	PUBLIC
		imgui
)

add_library(implot)

# C++ version
set_target_properties(implot
	PROPERTIES
		CXX_STANDARD 26
)

target_sources(implot
	PRIVATE
		${implot_SOURCE_DIR}/implot.cpp
		${implot_SOURCE_DIR}/implot_demo.cpp
		${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot
	PUBLIC
		${implot_SOURCE_DIR}
)

target_compile_definitions(implot
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

target_link_libraries(implot
	PUBLIC
		imgui
)

# set up glm C++ module as a library
get_target_property(GLM_INCLUDE_DIR glm::glm-header-only INTERFACE_INCLUDE_DIRECTORIES)

add_library(GlmModule)
target_sources(GlmModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${GLM_INCLUDE_DIR}
	FILES ${GLM_INCLUDE_DIR}/glm/glm.cppm
)
set_target_properties(GlmModule
	PROPERTIES
		CXX_STANDARD 26
)

target_compile_definitions(GlmModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		GLM_ENABLE_EXPERIMENTAL
		GLM_FORCE_DEPTH_ZERO_TO_ONE
		GLM_FORCE_RADIANS
		GLM_FORCE_SWIZZLE
)

target_link_libraries(GlmModule PUBLIC glm::glm-header-only)

# set up vma hpp C++ module as a library
add_library(VulkanMemoryAllocator-HppModule2)
target_sources(VulkanMemoryAllocator-HppModule2 PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${VulkanMemoryAllocator-Hpp_SOURCE_DIR}
	FILES ${VulkanMemoryAllocator-Hpp_SOURCE_DIR}/vk_mem_alloc.cppm
)

set_target_properties(VulkanMemoryAllocator-HppModule2
	PROPERTIES
		CXX_STANDARD 26
)

target_compile_definitions(VulkanMemoryAllocator-HppModule2
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		VMA_DYNAMIC_VULKAN_FUNCTIONS=0
		VMA_STATIC_VULKAN_FUNCTIONS=1
		VMA_ENABLE_VULKAN_HPP_MODULE
		VMA_ENABLE_STD_MODULE
)

target_link_libraries(VulkanMemoryAllocator-HppModule2
	PUBLIC
		VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
		VulkanHppModule

	# Uncomment when vma fixes https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/pull/514
	#INTERFACE
		GPUOpen::VulkanMemoryAllocator
)

# set up magic enum C++ module as a library
add_library(magic_enumModule)
target_sources(magic_enumModule PUBLIC
	FILE_SET CXX_MODULES
	BASE_DIRS ${magic_enum_SOURCE_DIR}
	FILES ${magic_enum_SOURCE_DIR}/module/magic_enum.cppm
)
set_target_properties(magic_enumModule
	PROPERTIES
		CXX_STANDARD 26
)
target_compile_definitions(magic_enumModule
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
	PUBLIC
		MAGIC_ENUM_USE_STD_MODULE
)
target_include_directories(magic_enumModule
	PRIVATE
		${magic_enum_SOURCE_DIR}/include
)

# Julia bindings

set(_lucuma_targets ${PROJECT_NAME})
if(TARGET ${PROJECT_NAME}-julia)
	list(APPEND _lucuma_targets ${PROJECT_NAME}-julia)
endif()

foreach(_lucuma_target IN LISTS _lucuma_targets)
	target_link_libraries(${_lucuma_target}
		PRIVATE
			EnTT::EnTT
			FastFloat::fast_float
			GlmModule
			PkgConfig::libraries
			Taskflow::Taskflow
			Threads::Threads
			VulkanMemoryAllocator-HppModule2
			imgui
			imgui-graphnode
			implot
			implot3d
			magic_enumModule
			nfd
			Tracy::TracyClient
	)

	if (XdgUtils_FOUND)
		target_link_libraries(${_lucuma_target}
			PRIVATE
				XdgUtils::BaseDir
		)
	else()
		target_compile_definitions(${_lucuma_target}
			PRIVATE
				NO_XDG
		)
	endif()

	target_include_directories(${_lucuma_target}
		PRIVATE
			${mdspan_SOURCE_DIR}/include
			${icon-fonts-cpp-headers_SOURCE_DIR}
	)
endforeach()
