cmake_minimum_required(VERSION 3.28)
project(vulkan-fdtd CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# --- Eigen (mode solver); system package, else a header search, else deps/ ---
find_package(Eigen3 3.4 QUIET NO_MODULE)
if(NOT TARGET Eigen3::Eigen)
	find_path(EIGEN3_INCLUDE_DIR Eigen/Dense
		PATH_SUFFIXES eigen3
		HINTS /opt/homebrew/include /usr/local/include)
	if(EIGEN3_INCLUDE_DIR)
		add_library(Eigen3::Eigen INTERFACE IMPORTED)
		set_target_properties(Eigen3::Eigen PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${EIGEN3_INCLUDE_DIR}")
	endif()
endif()

# --- Threads (Taskflow backend) ---
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# --- Vulkan (vulkan backend, optional); system package, else lib/header search ---
find_package(Vulkan QUIET)
if(NOT TARGET Vulkan::Vulkan)
	find_library(VK_LIB vulkan HINTS /opt/homebrew/lib /usr/local/lib /usr/lib)
	find_path(VK_INC vulkan/vulkan.h HINTS /opt/homebrew/include /usr/local/include /usr/include)
	if(VK_LIB AND VK_INC)
		add_library(Vulkan::Vulkan INTERFACE IMPORTED)
		set_target_properties(Vulkan::Vulkan PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${VK_INC}"
			INTERFACE_LINK_LIBRARIES "${VK_LIB}")
	endif()
endif()

# --- GLSL compute shaders -> SPIR-V -> embedded C arrays ---
find_program(GLSLANG glslangValidator HINTS /opt/homebrew/bin /usr/local/bin /usr/bin)
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fdtd/backends/vulkan/shaders)
set(SHADER_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_GEN_DIR})
set(SHADER_HEADERS)
if(GLSLANG)
	file(GLOB SHADER_SRCS ${SHADER_DIR}/*.comp)
	foreach(src ${SHADER_SRCS})
		get_filename_component(name ${src} NAME_WE)
		set(hdr ${SHADER_GEN_DIR}/${name}_comp_spv.h)
		add_custom_command(
			OUTPUT ${hdr}
			COMMAND ${GLSLANG} -V --target-env vulkan1.1
				--vn ${name}_comp_spv -o ${hdr} ${src}
			DEPENDS ${src}
			COMMENT "glsl ${name}.comp -> ${name}_comp_spv.h"
			VERBATIM)
		list(APPEND SHADER_HEADERS ${hdr})
	endforeach()
else()
	# No glslangValidator (e.g. cluster): use the committed prebuilt SPIR-V.
	message(STATUS "glslangValidator not found — using shaders/prebuilt/*.h")
	file(GLOB PREBUILT_SPV ${SHADER_DIR}/prebuilt/*_comp_spv.h)
	if(NOT PREBUILT_SPV)
		message(FATAL_ERROR "no glslangValidator and no shaders/prebuilt headers")
	endif()
	file(COPY ${PREBUILT_SPV} DESTINATION ${SHADER_GEN_DIR})
endif()
add_custom_target(vulkan-shaders DEPENDS ${SHADER_HEADERS})

# --- shared core (all but the entry points) ---
add_library(vulkan-core STATIC
	auxiliaries/funcs.cpp
	auxiliaries/modes.cpp
	auxiliaries/sources.cpp
	fdtd/utils.cpp
	fdtd/fw_sim.cpp
	vis/funcs.cpp
	fdtd/backends/sequential.cpp
	fdtd/backends/taskflow.cpp
	fdtd/backends/instantiator.cpp
	fdtd/backends/vulkan/vulkan.cpp
	fdtd/backends/vulkan/context.cpp
	fdtd/backends/vulkan/pipeline.cpp
	pipeline/0_loads.cpp
	pipeline/1_params.cpp
	pipeline/2_device.cpp
	pipeline/3_wg_source.cpp
	pipeline/4_fdtd_setup.cpp
	pipeline/7_forw_fdtd.cpp
)
add_dependencies(vulkan-core vulkan-shaders)

target_include_directories(vulkan-core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/deps
	${SHADER_GEN_DIR}
)

if(TARGET Eigen3::Eigen)
	target_link_libraries(vulkan-core PUBLIC Eigen3::Eigen)
endif()
target_link_libraries(vulkan-core PUBLIC Threads::Threads)
if(TARGET Vulkan::Vulkan)
	target_link_libraries(vulkan-core PUBLIC Vulkan::Vulkan)
	target_compile_definitions(vulkan-core PUBLIC VULKAN_BACKEND_AVAILABLE=1)
endif()

target_compile_definitions(vulkan-core PUBLIC
	VULKAN_FDTD_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_compile_options(vulkan-core PRIVATE -Wall -Wextra)

add_executable(vulkan-fdtd optimization.cpp)
target_link_libraries(vulkan-fdtd PRIVATE vulkan-core)
target_compile_options(vulkan-fdtd PRIVATE -Wall -Wextra)

# --- Ceviche design tests ---
foreach(challenge bend wdm mode_converter beam_splitter)
	add_executable(vulkan-${challenge}-test
		challenges/${challenge}/${challenge}_design_test.cpp)
	target_link_libraries(vulkan-${challenge}-test PRIVATE vulkan-core)
	target_compile_definitions(vulkan-${challenge}-test PRIVATE
		VULKAN_CHALLENGE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/challenges/${challenge}")
	target_compile_options(vulkan-${challenge}-test PRIVATE -Wall -Wextra)
endforeach()
