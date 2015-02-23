cmake_minimum_required(VERSION 2.8)
project(csv2h5)
find_package(HDF5 REQUIRED COMPONENTS CXX)

add_executable(csv2h5 main.cpp)
target_link_libraries(csv2h5 ${HDF5_LIBRARIES})
