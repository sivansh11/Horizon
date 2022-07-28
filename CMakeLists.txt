cmake_minimum_required(VERSION 3.10)

project(horizon)

if (WIN32)
set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.216.0")
elseif(UNIX)

endif()
find_package(Vulkan REQUIRED)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(horizon ${SRC_FILES})

target_include_directories(horizon
    PUBLIC src
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(horizon
    glfw
    glm
    ${Vulkan_LIBRARIES}
)

