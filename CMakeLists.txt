cmake_minimum_required(VERSION 3.30)
project(ColorRevamp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/Module)


set (SOURCES src/main.cpp)
set (RESOURCE src/Resource.rc)
set (HEADER src/resource.h)
set (MODULE src/module/ColorRevamp.ixx src/module/Helper.cpp)

source_group("Source files/Source" FILES ${SOURCES})
source_group("Source files/Header" FILES ${HEADER})
source_group("Source files/Resource" FILES ${RESOURCE})
source_group("Source files/Module" FILES ${MODULE})

set(CMAKE_TOOLCHAIN_FILE "C:/Program Files/vcpcg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADER} ${MODULE} ${RESOURCE})

#set(VCPKG_INSTALLED_DIR "C:/Program Files/vcpcg/vcpkg")
#set(OpenCV_DIR ${VCPKG_INSTALLED_DIR}/packages/opencv4_x64-windows/share/opencv4)
message("OpenCV_DIR ${VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")

find_package(OpenCV REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE opencv_ml opencv_dnn opencv_core opencv_flann)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})

include_directories(${OpenCV_INCLUDE_DIRS})

if (MSVC)
	set(CMAKE_CXX_FLAGS /experimental:module)
	target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
	set(CMAKE_CXX_MODULE_STD ON)
endif()

set_source_files_properties(${RESOURCE} PROPERITIES LANGUAGE RC)
