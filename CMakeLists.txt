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

cmake_minimum_required(VERSION 3.28.0)

project(fdtd-vulkan
	VERSION 0.0.0
	DESCRIPTION "Una GUI para fdtd"
	HOMEPAGE_URL "https://github.com/fdtd-lucuma/fdtd-vulkan"
)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

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

# Packages
find_package(bash-completion)

add_subdirectory(src) # Sources list
add_subdirectory(pkg) # Packaging

if(NOT USE_SYSTEM_LIBS)
	FetchContent_Declare(
		EnTT
		GIT_REPOSITORY https://github.com/skypjack/entt
		GIT_TAG        v3.15.0
		OVERRIDE_FIND_PACKAGE
	)

	FetchContent_MakeAvailable(EnTT)
endif()

# Shared libraries
pkg_check_modules(libraries
	REQUIRED IMPORTED_TARGET
		vulkan
		glfw3
)

find_package(VulkanMemoryAllocator REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::libraries

	INTERFACE # Header only
		GPUOpen::VulkanMemoryAllocator
		EnTT:EnTT
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
