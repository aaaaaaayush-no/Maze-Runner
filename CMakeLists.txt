cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0072 NEW)
project(MazeRunner LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

# GLM
find_path(GLM_INCLUDE_DIR glm/glm.hpp)
if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Install libglm-dev.")
endif()

# GLAD (bundled)
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Sources
set(SOURCES
    src/main.cpp
    src/Maze.cpp
    src/Player.cpp
    src/Renderer.cpp
    src/Minimap.cpp
    src/Collectible.cpp
    src/Shader.cpp
    src/TitleScreen.cpp
    src/HandRenderer.cpp
    src/SkyRenderer.cpp
    src/TorchLight.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${GLM_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Copy textures to build directory
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
