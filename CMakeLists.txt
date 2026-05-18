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

cmake_minimum_required(VERSION 4.0.3)

# Change this each version
# Read Help/dev/experimental.rst at CMake's repo for more info
if(CMAKE_MAJOR_VERSION EQUAL 4 AND CMAKE_MINOR_VERSION EQUAL 2)
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444" )
elseif(CMAKE_MAJOR_VERSION EQUAL 4 AND CMAKE_MINOR_VERSION EQUAL 3)
	set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b" )
endif()

# Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html
#add_compile_options($<$<CONFIG:DEBUG>:-D_GLIBCXX_DEBUG>)

# As of 2025 module : private doesn't compile with gcc
if(NOT DEFINED ENV{CC})
	set(CMAKE_C_COMPILER "clang")
endif()

if(NOT DEFINED ENV{CXX})
	set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_MODULE_STD 1)
add_compile_options(-Wp,-D_FORTIFY_SOURCE=0 -Wno-macro-redefined) # Quickfix: Disable fortification on release build types
add_compile_options($<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-rtti>) # No RTTI
add_compile_definitions(vulkan_hpp=vulkan) # vulkan_hpp was renamed to vulkan

if(BUILD_FOR_TERMUX)
	add_compile_definitions(__BIONIC_CTYPE_INLINE=inline)
endif()

project(fdtd-lucuma
	VERSION 0.2.0
	DESCRIPTION "Una GUI para fdtd"
	HOMEPAGE_URL "https://github.com/fdtd-lucuma/fdtd-lucuma"
)

include(FetchContent)
include(GNUInstallDirs)
include(FindPkgConfig)
include(CPack)

# ./cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(SpirvTarget)

# The program itself
add_executable(${PROJECT_NAME})

# C++ version
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 26
)

add_subdirectory(src) # Sources list
add_subdirectory(pkg) # Packaging
add_subdirectory(deps) # Hardcoded dependencies
add_subdirectory(share)

# Macros
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		$<$<NOT:$<CONFIG:DEBUG>>:NDEBUG>
)

# Default flags
target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -fdiagnostics-color=always>
)

# Bash completion
find_package(bash-completion REQUIRED)

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
