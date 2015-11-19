message(STATUS cmake-lib)
set(SHOWBEAR_SOURCES
    glslprogram.cc
    glutils.cc
    model.cc
    )

include_directories(${CMAKE_SOURCE_DIR}/source/)
include_directories(${CMAKE_SOURCE_DIR}/source/lib)
include_directories(${CMAKE_SOURCE_DIR}/source/thirdparty/glm)

add_library(showbear STATIC ${SHOWBEAR_SOURCES})

target_link_libraries(showbear ${GIANTRAT_LIBS} )

add_dependencies(showbear glfw)
add_dependencies(showbear glm)
