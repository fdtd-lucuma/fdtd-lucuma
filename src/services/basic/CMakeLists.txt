# Una GUI para fdtd
# Copyright © 2025 Otreblan
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

target_sources(${PROJECT_NAME}
	PRIVATE
		argument_parser.cpp
		file_reader.cpp
		instantiations.cpp
		settings.cpp
	PRIVATE
		FILE_SET fdtd
		TYPE CXX_MODULES
		FILES
			argument_parser.cppm
			basic.cppm
			file_reader.cppm
			path.cppm
			settings.cppm
			settings_enums.cppm
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(FDTD_DATA_DIR "${CMAKE_BINARY_DIR}/share")
else()
	set(FDTD_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
endif()

target_include_directories(${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(path_config.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/path_config.hpp
	@ONLY
)
