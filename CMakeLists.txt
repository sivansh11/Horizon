cmake_minimum_required(VERSION 3.10)

project(horizon)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(horizon ${SRC_FILES})

target_include_directories(horizon
    PUBLIC src
)

target_link_libraries(horizon
    glfw
    glm
    vulkan
    dl
    pthread
    X11
    Xxf86vm
    Xrandr
    Xi
)
