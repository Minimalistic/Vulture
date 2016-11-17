# CMakeLists files in this project can
# refer to the root source directory of the project as ${VulkanTest_SOURCE_DIR} and
# to the root binary directory of the project as ${VulkanTest_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (VulkanTest)
add_executable(VulkanTest test.cpp)

# Enable C++11 support
target_compile_features(VulkanTest PRIVATE cxx_range_for)

# Include Vulkan headers
include_directories("$ENV{VULKAN_SDK}/include")

# Link Vulkan
find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
target_link_libraries(VulkanTest ${Vulkan_LIBRARY})

# Link Assimp
find_package(ASSIMP REQUIRED)

# Link Pthreads
find_package(Threads REQUIRED)

