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

cmake_minimum_required(VERSION 3.30.0)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT DEFINED ENV{CC})
	set(CMAKE_C_COMPILER "/usr/bin/clang")
endif()

if(NOT DEFINED ENV{CXX})
	set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
endif()

project(fdtd-vulkan
	VERSION 0.0.0
	DESCRIPTION "Una GUI para fdtd"
	HOMEPAGE_URL "https://github.com/fdtd-lucuma/fdtd-vulkan"
)

include(FetchContent)
include(GNUInstallDirs)
include(FindPkgConfig)
include(CPack)

# The program itself
add_executable(${PROJECT_NAME})

# C++ version
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 23
)

add_subdirectory(src) # Sources list
add_subdirectory(pkg) # Packaging
add_subdirectory(deps) # Hardcoded dependencies

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

# Macros
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		$<$<CONFIG:DEBUG>:VK_DEBUG>
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
		VMA_DYNAMIC_VULKAN_FUNCTIONS=1
		VMA_STATIC_VULKAN_FUNCTIONS=0
		GLM_FORCE_DEPTH_ZERO_TO_ONE
		GLM_FORCE_RADIANS
		GLFW_INCLUDE_VULKAN
		VMA_ENABLE_VULKAN_HPP_MODULE
)

# Default flags
target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Bash completion
file(WRITE ${CMAKE_BINARY_DIR}/bash-completion/${PROJECT_NAME}
	"complete -F _longopt ${PROJECT_NAME}"
)

install(FILES
	${CMAKE_BINARY_DIR}/bash-completion/${PROJECT_NAME}
	DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR}/
)

# Install target
install(TARGETS ${PROJECT_NAME}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
