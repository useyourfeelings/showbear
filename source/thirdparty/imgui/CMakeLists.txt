# xc add

message(STATUS imgui-cmake) 
set( IMGUI_SOURCES
    examples/opengl3_example/imgui_impl_glfw_gl3.cpp
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    )

include_directories("./")
include_directories(${CMAKE_SOURCE_DIR}/source)
add_library(imgui STATIC ${IMGUI_SOURCES})