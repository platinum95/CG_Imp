# Make all lab executables
cmake_minimum_required( VERSION 2.8.11 )
set( OpenGL_GL_PREFERENCE "GLVND" )

subdirs( CG_Engine )

add_subdirectory( CG_Implementation )
