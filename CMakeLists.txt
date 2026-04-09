cmake_minimum_required(VERSION 3.10)
project(benchmark)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(ARCH ${CMAKE_SYSTEM_PROCESSOR})

MESSAGE("-- CPU architecture: ${ARCH}")

# 构建类型选项，默认为Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type: Debug or Release" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# 设置C++标准和编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -Wall -Wextra -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -Wall -Wextra -O0")
    # add_compile_definitions(ENABLE_DEBUG_MODE=1)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -O3 -s")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O3 -s -ffunction-sections -fdata-sections")
    # add_compile_definitions(ENABLE_DEBUG_MODE=0)
endif()

# 链接时优化
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")


# 安装路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/deploy)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)

# 3RD 包含目录
set(3RD_INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/3rd/include/iceoryx/v2.0.2"
    "${CMAKE_SOURCE_DIR}/3rd/include"
    "${CMAKE_SOURCE_DIR}/3rd/include/ddscxx"
)

# 架构检测和库路径设置 -- TODO: ${ARCH}
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "Building for ARM64/aarch64 architecture")
    set(3RD_LIBRARY_DIRS
        "${CMAKE_SOURCE_DIR}/3rd/lib"
    )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Building for x86_64 architecture")
    set(3RD_LIBRARY_DIRS
        "${CMAKE_SOURCE_DIR}/3rd/lib"
    )
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()


# 系统链接库
# set(DDS_RUN_LIBS ddsc ddscxx rt pthread iceoryx_binding_c iceoryx_hoofs iceoryx_posh)
set(DDS_RUN_LIBS ddsc ddscxx rt pthread)


add_subdirectory(examples)
add_subdirectory(utils/bcli)




## Install thirdparty .so libraries from 3RD_LIBRARY_DIRS (per-architecture)
## Use PROGRAMS to preserve symlinks and file permissions
foreach(_lib_dir IN LISTS 3RD_LIBRARY_DIRS)
    if(EXISTS "${_lib_dir}")
        # Get all .so files (including symlinks) in the directory
        file(GLOB _so_files "${_lib_dir}/*.so" "${_lib_dir}/*.so.*")
        if(_so_files)
            install(
                FILES ${_so_files}
                DESTINATION lib
            )
        endif()
    endif()
endforeach()


message(STATUS "Build configuration:")
message(STATUS "  Target: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "  Debug mode: ${ENABLE_DEBUG_MODE}")
message(STATUS "  Optimized for embedded systems")

