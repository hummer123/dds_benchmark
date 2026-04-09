cmake_minimum_required(VERSION 3.10)
project(bcli CXX)


#//-------------------------------------------------------#
## set include and link directories
include_directories(
    ${3RD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/utils/bcli
    ${CMAKE_SOURCE_DIR}/msg
)

# set library search path for 3rd party libraries and msg
link_directories(
    ${3RD_LIBRARY_DIRS}
)


# Create bcli executable
add_executable(${PROJECT_NAME} 
    bcli_main.cpp
    bcli_netload.cpp
    bcli_cputime.cpp
    ${CMAKE_SOURCE_DIR}/msg/test_data.cpp

    adapters/adapter_cyclonedds.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${DDS_RUN_LIBS})
#//-------------------------------------------------------#


# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)



