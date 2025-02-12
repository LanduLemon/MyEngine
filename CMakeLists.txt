cmake_minimum_required(VERSION 3.13)
project(myengine)
# 递归查找 src 目录下的所有 .cpp 文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 检查是否找到任何源文件
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found in src directory")
endif()
add_executable(myengine ${SOURCES})

target_include_directories(myengine PRIVATE "C:/VulkanSDK/1.3.283.0/include"
                                            "C:/VulkanSDK/1.3.283.0/Third-Party/include"
                                            "${CMAKE_SOURCE_DIR}/include"
                                            "C:/workspace_win/glfw-3.4/include/"
                                            "${CMAKE_SOURCE_DIR}/libs/tinyobjloader"
                                            )

target_link_libraries(myengine PRIVATE "C:/VulkanSDK/1.3.283.0/Lib/vulkan-1.lib"
                                       "C:/workspace_win/glfw-3.4/build/src/Release/glfw3.lib"
                                       )

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")

add_custom_command(TARGET myengine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:myengine>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:myengine>/models")

add_custom_target(run_compile_bat
    COMMAND "${CMAKE_SOURCE_DIR}/compile.bat"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "excute compile.bat")
add_dependencies(myengine run_compile_bat)