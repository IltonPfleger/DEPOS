cmake_minimum_required (VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
project (smartdata)
add_definitions(-DTEMP_IMAGE_SOURCE="${CMAKE_SOURCE_DIR}/../external/carlav2/temporary_data/CAMERA_IMAGE0.raw")

# ============================================================
# dlprimitives
# ============================================================

set(DLPRIM_ROOT ${CMAKE_SOURCE_DIR}/lib/dlprimitives)
set(DLPRIM_LIB_DIR ${DLPRIM_ROOT}/build)

add_library(dlprim_core SHARED IMPORTED)
set_target_properties(dlprim_core PROPERTIES
    IMPORTED_LOCATION ${DLPRIM_LIB_DIR}/libdlprim_core.so
    INTERFACE_INCLUDE_DIRECTORIES ${DLPRIM_ROOT}/include
)

add_library(dlprim_onnx SHARED IMPORTED)
set_target_properties(dlprim_onnx PROPERTIES
    IMPORTED_LOCATION ${DLPRIM_LIB_DIR}/libdlprim_onnx.so
    INTERFACE_INCLUDE_DIRECTORIES ${DLPRIM_ROOT}/include
)

add_library(dlprim SHARED IMPORTED)
set_target_properties(dlprim PROPERTIES
    IMPORTED_LOCATION ${DLPRIM_LIB_DIR}/libdlprim.so
    INTERFACE_INCLUDE_DIRECTORIES ${DLPRIM_ROOT}/include
)

set_property(TARGET dlprim PROPERTY
    INTERFACE_LINK_LIBRARIES
        dlprim_core
        dlprim_onnx
)

# ============================================================

add_definitions(-DVVD_SOURCE_DIR="${CMAKE_SOURCE_DIR}/../external/AV_Modules_SD/visual_vibration_detection_cpp")
add_subdirectory("../external/AV_Modules_SD/visual_vibration_detection_cpp" VVD_build)


add_definitions(-DRSCD_SOURCE_DIR="${CMAKE_SOURCE_DIR}/../external/AV_Modules_SD/road_surface_condition_detection_cpp")
add_subdirectory("../external/AV_Modules_SD/road_surface_condition_detection_cpp" RSCD_build)


if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Valid values: Release, Debug" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
if(CMAKE_BUILD_TYPE MATCHES Debug)
	ADD_DEFINITIONS(-D_DEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
else(CMAKE_BUILD_TYPE MATCHES Debug)
	message(FATAL_ERROR "Select build type")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

include_directories (
	${smartdata_SOURCE_DIR}/.
	${smartdata_SOURCE_DIR}/include
)

set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3 -Wfatal-errors -Wuninitialized -fno-strict-aliasing -fomit-frame-pointer")

add_executable (smartdata
	app/${APPLICATION}.cpp
	src/network/tstp/locator.cc
	src/network/tstp/manager.cc
	src/network/tstp/router.cc
	src/network/tstp/security.cc
	src/network/tstp/timekeeper.cc
	src/network/tstp/tstp.cc
	src/network/tstp/tstp_init.cc
	src/system/thread.cc
	src/utility/aes.cc
	src/utility/bignum.cc
	src/utility/ostream.cc
	src/utility/random.cc
)

find_package(Threads REQUIRED)
target_link_libraries(smartdata PRIVATE RSCD VVD dlprim Threads::Threads)

set_target_properties(smartdata PROPERTIES
    BUILD_RPATH "${DLPRIM_LIB_DIR}"
)

option(USE_CAMERA_HARDWARE "If camera hardware support should be compiled" OFF)
option(USE_LIDAR_HARDWARE "If lidar hardware support should be compiled" OFF)
option(USE_IMU_HARDWARE "If imu hardware support should be compiled" OFF)
option(USE_CLOUD_INTEGRATION "If IoT Platform sender support should be compiled" OFF)
option(NO_DATA_SOURCE "If imu hardware support should be compiled" OFF)

if (NO_DATA_SOURCE)
	add_definitions(-DNO_DATA_SOURCE)
endif(NO_DATA_SOURCE)


if (USE_CLOUD_INTEGRATION)
	add_definitions(-DCLOUD_INTEGRATION)
	find_package(OpenSSL REQUIRED)
	target_link_libraries(smartdata PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif(USE_CLOUD_INTEGRATION)

if (USE_LIDAR_HARDWARE)
	add_definitions(-DLIDAR_HARDWARE)
	if (USE_LIDAR_VIEWER)
		add_definitions(-DLIDAR_VIEWER)
		set(OpenGL_GL_PREFERENCE LEGACY)
		find_package(OpenGL REQUIRED)
		find_package(GLUT REQUIRED)
		target_link_libraries(smartdata PRIVATE OpenGL::GL GLUT::GLUT)
	endif (USE_LIDAR_VIEWER)
endif(USE_LIDAR_HARDWARE)

if (USE_IMU_HARDWARE)
	add_definitions(-DIMU_HARDWARE)
	add_definitions(-DGNSS_HARDWARE)
	find_package(OpenZen CONFIG REQUIRED)
	target_link_libraries (smartdata PRIVATE OpenZen::OpenZen ${ADDITIONAL_LIBS} pthread rt )
endif(USE_IMU_HARDWARE)

if(USE_CAMERA_HARDWARE)
	add_definitions(-DCAMERA_HARDWARE)
	find_package(depthai CONFIG REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(OpenCV REQUIRED)
	include_directories(${OpenCV_INCLUDE_DIRS})
	target_link_libraries (smartdata PRIVATE depthai::opencv ${ADDITIONAL_LIBS} ${OpenCV_LIBS} OpenSSL::SSL OpenSSL::Crypto pthread rt)
	if (USE_CAMERA_VIEWER)
		add_definitions(-DCAMERA_VIEWER)
	endif(USE_CAMERA_VIEWER)
endif(USE_CAMERA_HARDWARE)
