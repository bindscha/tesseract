cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_COMPILER /usr/bin/gcc)

project(tesseract)
cmake_policy(VERSION 3.5.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FILE_OFFSET_BITS=64  -D_FILE_OFFSET_BITS=64 -pthread -fms-extensions -ggdb3 -std=c++11 -pthread -fopenmp -U_FORTIFY_SOURCE -O3 -fno-stack-protector ")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories("${PROJECT_SOURCE_DIR}")
#add_executable(tesseract graph.cpp engine_one.hpp libtesseract.h tesseract.hpp tesseract.cpp)
add_executable(libtesseract graph.cpp  libtesseract.h embedding.hpp engine_one.hpp libtesseract.cpp tesseract_driver.cpp scala_algos.h Bitmap.h)
#target_link_libraries(tesseract)
target_link_libraries(libtesseract)