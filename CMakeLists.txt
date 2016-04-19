#-------------------------------------------------------------------------------
# build the pipes examples
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)
project(pipes)

set(CMAKE_CXX_FLAGS "-std=c++14")

add_executable(run_examples examples.cxx)
