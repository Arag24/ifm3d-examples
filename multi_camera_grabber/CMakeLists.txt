cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)

project(ex-multi_camera_grabber CXX)

# Global compiler flags
set(CMAKE_BUILD_TYPE Release) # Release or Debug
set(CMAKE_CXX_EXTENSIONS OFF) # OFF -> -std=c++14, ON -> -std=gnu++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED true)

##############################################################
## finding the ifm3d lib.
##############################################################
find_package(ifm3d 0.11.0 CONFIG
  REQUIRED COMPONENTS camera framegrabber image
  )

add_executable(ex-multi_camera_grabber ex-multi_camera_grabber.cpp)
target_link_libraries(ex-multi_camera_grabber
                    ifm3d::camera
                    ifm3d::framegrabber
                    ifm3d::image
                    )
