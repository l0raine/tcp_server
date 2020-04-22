cmake_minimum_required(VERSION 3.10)

project(server)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")
endif (UNIX)

set(source_dir "${PROJECT_SOURCE_DIR}/src")

file(GLOB source_files "${source_dir}/*.cpp")
file(GLOB header_files "${source_dir}/*.h")

add_executable(
	server
	${source_files}
	${header_files}
)

add_subdirectory(extern/spdlog)

target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/extern)
target_link_libraries(server PRIVATE spdlog)