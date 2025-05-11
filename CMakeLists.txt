cmake_minimum_required(VERSION 3.30)
project(openvr)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
set(GLEW_INCLUDE_DIR vendored/glew-2.2.0/include)
set(GLEW_LIBRARY "C:/Users/aidan/OneDrive/Documents/GitHub/openvr/vendored/glew-2.2.0/lib/Release/x64/glew32.lib")
add_subdirectory(vendored/glm EXCLUDE_FROM_ALL)
set(glm_SOURCE_DIR vendored/glm)
include_directories(${GLEW_INCLUDE_DIR})
link_libraries(${GLEW_LIBRARY})
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
add_executable(openvr main.cpp
        render/renderStack.cpp
        render/renderStack.h
        render/renderable.cpp
        render/renderable.h
        render/renderNode.cpp
        render/renderNode.h
        render/RenderDummy.cpp
        render/RenderDummy.h
        render/camera.cpp
        render/camera.h
        render/3d/cube.cpp
        render/3d/cube.h
        Pose3d.cpp
        Pose3d.h
        Vector3d.cpp
        Vector3d.h
        render/3d/Vertex3d.cpp
        render/3d/Vertex3d.h
        render/3d/Face3d.cpp
        render/3d/Face3d.h
        render/SDLConfig.cpp
        render/SDLConfig.h
        render/renderableRT.cpp
        render/renderableRT.h
        render/3d/renderUtil.cpp
        render/3d/renderUtil.h
        render/GPU/gpu.cpp
        render/GPU/gpu.h
        render/3d/SerializedObject.cpp
        render/3d/SerializedObject.h
        Vector2d.cpp
        Vector2d.h)
target_include_directories(openvr PRIVATE ${glm_SOURCE_DIR})
target_link_libraries(openvr PRIVATE SDL3::SDL3 ${OpenCL_LIBRARIES} opengl32 gdi32 ${GLEW_LIBRARY} glm)