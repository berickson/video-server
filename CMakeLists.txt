cmake_minimum_required(VERSION 2.8.2)
set(CMAKE_CXX_STANDARD 14)

set(CPP_FILES main.cpp)

find_package(OpenCV REQUIRED)
add_executable(video-server ${CPP_FILES})
target_include_directories(video-server PRIVATE external/web-server)
target_compile_options(video-server PRIVATE -Wno-psabi)
target_link_libraries(video-server ${OpenCV_LIBS} pthread)
