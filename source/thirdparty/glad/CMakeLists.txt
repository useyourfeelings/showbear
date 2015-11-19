# xc add

message(STATUS glad-cmake) 
set( GLAD_SOURCES
    glad.c
    )

include_directories("./")
add_library(glad STATIC ${GLAD_SOURCES})