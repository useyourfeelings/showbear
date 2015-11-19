set(DEMO_NAME stenciltest)
message(STATUS cmake-${DEMO_NAME})
set( SOURCES
    stenciltest.cc
    ${CMAKE_SOURCE_DIR}/source/data/shaders/stenciltest.vs
    ${CMAKE_SOURCE_DIR}/source/data/shaders/stenciltest.fs
)

include_directories(${CMAKE_SOURCE_DIR}/source/)
include_directories(${CMAKE_SOURCE_DIR}/source/lib)
include_directories(${CMAKE_SOURCE_DIR}/source/thirdparty/glm)
include_directories(${CMAKE_SOURCE_DIR}/source/thirdparty/imgui/examples/opengl3_example)

add_executable(${DEMO_NAME} ${SOURCES} )

target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/lib/libshowbear.a)
target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/glad/libglad.a)
target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/glfw/src/libglfw3.a)
target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/assimp/code/libassimp.a)
target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/imgui/libimgui.a)
target_link_libraries(${DEMO_NAME} ${OPENGL_gl_LIBRARY})

add_dependencies(${DEMO_NAME} showbear)

add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin/data)
add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/source/data ${CMAKE_SOURCE_DIR}/bin/data)
add_custom_command(TARGET ${DEMO_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_SOURCE_DIR}/bin)
