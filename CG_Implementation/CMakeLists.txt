
cmake_minimum_required( VERSION 2.8.11 )
project( CG_Engine )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O3")
find_package(CG_Engine REQUIRED)

file(GLOB_RECURSE SOURCES "./*.cpp")

#include_directories( "../CG_Engine/include" )
#include_directories( "../CG_Engine/CG_Engine" )

set( OpenGL_GL_PREFERENCE "GLVND" )

add_executable( cg_imp ${SOURCES} ) 
target_link_libraries( cg_imp CG_Engine glfw assimp dl )
