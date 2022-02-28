
cmake_minimum_required( VERSION 3.16 )

project( SWater )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

## C++14 is okay here, could prolly work in C++11 too
set( CMAKE_CXX_STANDARD 14 )

## I have a habit of setting a root variable cuz' I'm lazy to type CMAKE_CURRENT_SOURCE_DIR every time
## In projects like these, which aren't meant to be used as dependencies, I prefix stuff with THE_,
## cuz' it's THE stuff, there won't be any other
set( THE_ROOT ${CMAKE_CURRENT_SOURCE_DIR} )

## Set up SDL2, OpenGL, glew, glm, stb_image and ImGui

## SDL2
if( UNIX )
    find_package( SDL2 REQUIRED )
elseif( WIN32 )
    ## Note that I've only included 64-bit SDL2 binaries
    set( SDL2_INCLUDE_DIRS
        ${THE_ROOT}/extern/SDL2/include )

    set( SDL2_LIBRARIES
        ${THE_ROOT}/extern/SDL2/lib/SDL2.lib
        ${THE_ROOT}/extern/SDL2/lib/SDL2main.lib )
else()
    message( FATAL_ERROR "This platform is not supported" )
endif()

## OpenGL
find_package( OpenGL REQUIRED )

## glew
set( GLEW_INCLUDE_DIR
    ${THE_ROOT}/extern/glew/include )
set( GLEW_SOURCES
    ${THE_ROOT}/extern/glew/src/glew.c )

## stb_image
set( STB_IMAGE_INCLUDE_DIR 
    ${THE_ROOT}/extern/stb_image )

## Set up our main thing

set( THE_SOURCES 
    ${THE_ROOT}/src/Main.cpp 
    ${THE_ROOT}/src/IApp.hpp 
    ${THE_ROOT}/src/App.hpp 
    ${THE_ROOT}/src/App.cpp 
    ${THE_ROOT}/src/TextureProvider.hpp 
    ${THE_ROOT}/src/TextureProvider.cpp 
    ${GLEW_SOURCES} ## glew will be built into this directly 
    )

## Folder organisation
source_group( TREE ${THE_ROOT} FILES ${THE_SOURCES} )

## The .exe
add_executable( SWater ${THE_SOURCES} )

## Include dirs
target_include_directories( SWater PRIVATE
    ${THE_ROOT}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${STB_IMAGE_INCLUDE_DIR} )

## Link against SDL2 libs
target_link_libraries( SWater PRIVATE ${SDL2_LIBRARIES} opengl32 )

## Output here
install( TARGETS SWater
    RUNTIME DESTINATION ${THE_ROOT}/bin/
    LIBRARY DESTINATION ${THE_ROOT}/bin/ )

## On Windows, copy SDL2.dll and the .pdb
if( WIN32 )
    install( FILES ${THE_ROOT}/extern/SDL2/lib/SDL2.dll
        DESTINATION ${THE_ROOT}/bin/ )

    install( FILES $<TARGET_PDB_FILE:SWater> DESTINATION ${BTX_ROOT}/bin/ OPTIONAL )
endif()
