cmake_minimum_required(VERSION 3.10)
project(Listener)


## set include and link directories
include_directories(
    ${3RD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/msg
)

# set library search path for 3rd party libraries and msg
link_directories(
    ${3RD_LIBRARY_DIRS}
    ${CMAKE_SOURCE_DIR}/msg
)

#-------------------------------------------------------#


## c++ raw_talker
add_executable(raw_talker
    listener/raw_talker.cpp
    ${CMAKE_SOURCE_DIR}/msg/test_data.cpp
)

add_executable(raw_listener
    listener/raw_listener.cpp
    ${CMAKE_SOURCE_DIR}/msg/test_data.cpp
)

target_link_libraries(raw_talker PRIVATE ${DDS_RUN_LIBS})
target_link_libraries(raw_listener PRIVATE ${DDS_RUN_LIBS})

# 安装规则
install(TARGETS raw_talker raw_listener
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)


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

