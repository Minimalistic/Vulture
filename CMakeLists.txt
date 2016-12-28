# CMakeLists files in this project can
# refer to the root source directory of the project as ${Vulture_SOURCE_DIR} and
# to the root binary directory of the project as ${Vulture_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (Vulture)
set (Vulture_VERSION_MAJOR 1)
set (Vulture_VERSION_MINOR 0)

# Include custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Enable C++11 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_DIR src)

include_directories(include)

add_library(Utility ${SOURCE_DIR}/util.cpp)
add_library(Allocator ${SOURCE_DIR}/allocator.cpp)

IF(WIN32)
add_executable(Vulture WIN32 ${SOURCE_DIR}/test.cpp)
ELSE(WIN32)
add_executable(Vulture ${SOURCE_DIR}/test.cpp)
ENDIF(WIN32)

target_link_libraries(Vulture Utility)
target_link_libraries(Vulture Allocator)

# Include Vulkan headers
include_directories("$ENV{VULKAN_SDK}/include")

# Link threading libraries
find_package(Threads REQUIRED)
target_link_libraries(Vulture ${CMAKE_THREAD_LIBS_INIT})

IF(WIN32)

  # Link Vulkan
  find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)
  target_link_libraries(Vulture ${Vulkan_LIBRARY})

ELSE(WIN32)

  # Link Vulkan
  find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
  target_link_libraries(Vulture ${Vulkan_LIBRARY})

  # Link Xlib
  find_package(X11 REQUIRED)
  target_link_libraries(Vulture ${X11_LIBRARIES})
  include_directories(${X11_INCLUDE_DIR})

  # Link XCB
  find_package(XCB REQUIRED)
  target_link_libraries(Vulture ${XCB_LIBRARIES})

ENDIF(WIN32)
