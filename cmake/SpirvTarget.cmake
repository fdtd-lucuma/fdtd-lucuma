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

function(add_spirv_target)
	set(options)
	set(oneValueArgs TARGET DESTINATION)
	set(multiValueArgs SOURCES)

	cmake_parse_arguments(ARG
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN}
	)

	if(NOT IS_EXECUTABLE ${SLANG_EXECUTABLE})
		message(FATAL_ERROR "Couldn't find slangc in $SLANG_EXECUTABLE. Did you call find_package(slang)?")
	endif()

	foreach(SOURCE IN LISTS ARG_SOURCES)
		get_filename_component(SOURCE_NAME "${SOURCE}" NAME_WLE)
		set(BINARY_NAME "${SOURCE_NAME}.spv")

		add_custom_command(
			OUTPUT "${BINARY_NAME}"
			DEPENDS "${SOURCE}"
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			COMMAND "${SLANG_EXECUTABLE}" "${SOURCE}" -profile glsl_450 -target spirv -o "${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}" -entry main
			COMMENT "${SOURCE} -> ${BINARY_NAME}"
		)

		list(APPEND BINARIES "${BINARY_NAME}")
	endforeach()

	install(
		FILES $<LIST:TRANSFORM,BINARIES,PREPEND,"${CMAKE_CURRENT_BINARY_DIR}/">
		DESTINATION "${ARG_DESTINATION}"
	)

	add_custom_target("${ARG_TARGET}" ALL
		DEPENDS "${BINARIES}"
	)

endfunction()
