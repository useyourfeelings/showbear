set(DEMO_NAME basicads)
message(STATUS cmake-${DEMO_NAME})
set( SOURCES
    basicads.cc
    ${CMAKE_SOURCE_DIR}/source/data/shaders/basicads.vs
    ${CMAKE_SOURCE_DIR}/source/data/shaders/basicads.fs
)

include_directories(${CMAKE_SOURCE_DIR}/source/)
include_directories(${CMAKE_SOURCE_DIR}/source/lib)
include_directories(${CMAKE_SOURCE_DIR}/source/thirdparty/glm)
include_directories(${CMAKE_SOURCE_DIR}/source/thirdparty/imgui/examples/opengl3_example)

add_executable(${DEMO_NAME} ${SOURCES})

if(MSVC)
    set(LIBRARIES_TO_LINK
    optimized ${CMAKE_BINARY_DIR}/source/lib/release/showbear.lib
    optimized ${CMAKE_BINARY_DIR}/source/thirdparty/glad/release/glad.lib
    optimized ${CMAKE_BINARY_DIR}/source/thirdparty/glfw/src/release/glfw3.lib
    optimized ${CMAKE_BINARY_DIR}/source/thirdparty/assimp/code/release/assimp-vc130-mt.lib
    optimized ${CMAKE_BINARY_DIR}/source/thirdparty/imgui/release/imgui.lib

    debug ${CMAKE_BINARY_DIR}/source/lib/debug/showbeard.lib
    debug ${CMAKE_BINARY_DIR}/source/thirdparty/glad/debug/gladd.lib
    debug ${CMAKE_BINARY_DIR}/source/thirdparty/glfw/src/debug/glfw3d.lib
    debug ${CMAKE_BINARY_DIR}/source/thirdparty/assimp/code/debug/assimp-vc130-mtd.lib
    debug ${CMAKE_BINARY_DIR}/source/thirdparty/imgui/debug/imguid.lib
        )

    target_link_libraries(${DEMO_NAME} ${LIBRARIES_TO_LINK})

    add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/source/demos/${DEMO_NAME}/data)
    # vs2015 work dir is $(ProjectDir) by default
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bin/data ${CMAKE_BINARY_DIR}/source/demos/${DEMO_NAME}/data)
else()
    target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/lib/libshowbear.a)
    target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/glad/libglad.a)
    target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/glfw/src/libglfw3.a)
    target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/assimp/code/libassimp.a)
    target_link_libraries(${DEMO_NAME} ${CMAKE_BINARY_DIR}/source/thirdparty/imgui/libimgui.a)

    add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin/data)
    add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/source/data ${CMAKE_SOURCE_DIR}/bin/data)
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DEMO_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_SOURCE_DIR}/bin)
    add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data)
    add_custom_command(TARGET ${DEMO_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/source/data ${CMAKE_BINARY_DIR}/data)

endif()

target_link_libraries(${DEMO_NAME} ${OPENGL_gl_LIBRARY})

add_dependencies(${DEMO_NAME} showbear)

