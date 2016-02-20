cmake_minimum_required(VERSION 2.8)
set(OpenCV_DIR "/usr/local/opencv-3.0.0/release")
project( visualStudio )
find_package( OpenCV REQUIRED )
add_executable( main.exe main.cpp )
ADD_DEFINITIONS(
    -std=c++11
)
target_link_libraries( main.exe ${OpenCV_LIBS} )
