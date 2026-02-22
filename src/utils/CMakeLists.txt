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

target_sources(${PROJECT_NAME}
	PRIVATE
		copy.cpp
		exceptions.cpp
		imgui_graphnode_edge_writer.cpp
		injector.cpp
		instantiations.cpp
		stream_edge_writer.cpp
	PRIVATE
		FILE_SET fdtd
		TYPE CXX_MODULES
		FILES
			alias.cppm
			backend.cppm
			copy.cppm
			dims.cppm
			exceptions.cppm
			field.cppm
			imgui_graphnode_edge_writer.cppm
			injector.cppm
			mdspan.cppm
			plane.cppm
			precision.cppm
			print.cppm
			save_as.cppm
			stream_edge_writer.cppm
			utils.cppm
			vector_component.cppm
)

add_subdirectory(imgui)
add_subdirectory(vulkan)
