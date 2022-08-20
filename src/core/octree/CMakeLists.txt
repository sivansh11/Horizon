cmake_minimum_required(VERSION 3.10)

project(raytracer)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}

)
