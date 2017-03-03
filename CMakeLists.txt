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

set(DEFAULT_BUFFERS         13)
set(DEFAULT_IMAGES          21)
set(DEFAULT_COMMAND_BUFFERS 5)
set(DEFAULT_GLSL            450)
set(DEFAULT_INSTANCES       2)

IF(BUFFERS)
  set(NUM_BUFFERS ${BUFFERS})
ELSE(BUFFERS)
  set(NUM_BUFFERS ${DEFAULT_BUFFERS})
ENDIF(BUFFERS)

IF(IMAGES)
  set(NUM_IMAGES ${IMAGES})
ELSE(IMAGES)
  set(NUM_IMAGES ${DEFAULT_IMAGES})
ENDIF(IMAGES)

IF(COMMAND_BUFFERS)
  set(NUM_COMMAND_BUFFERS ${COMMAND_BUFFERS})
ELSE(COMMAND_BUFFERS)
  set(NUM_COMMAND_BUFFERS ${DEFAULT_COMMAND_BUFFERS})
ENDIF(COMMAND_BUFFERS)

IF(GLSL)
  set(GLSL_VERSION ${GLSL})
ELSE(GLSL)
  set(GLSL_VERSION ${DEFAULT_GLSL})
ENDIF(GLSL)

IF(INSTANCES)
  set(NUM_INSTANCES ${INSTANCES})
ELSE(INSTANCES)
  set(NUM_INSTANCES ${DEFAULT_INSTANCES})
ENDIF(INSTANCES)

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shader")
set(SHADER_COMPILER "$ENV{VULKAN_SDK}/bin/glslangValidator")

add_definitions(-DBUFFER_COUNT=${NUM_BUFFERS})
add_definitions(-DIMAGE_COUNT=${NUM_IMAGES})
add_definitions(-DCOMMAND_BUFFER_COUNT=${NUM_COMMAND_BUFFERS})
add_definitions(-DINSTANCE_COUNT=${NUM_INSTANCES})

set(CPP_SOURCE_DIR "src/main")

set (GLM_VERSION 0.9.8.4)
set (TINYOBJLOADER_VERSION 1.0.5)

include_directories(include)
include_directories("$ENV{VULKAN_SDK}/include")
include_directories(lib/glm-${GLM_VERSION})
include_directories(lib/tinyobjloader-${TINYOBJLOADER_VERSION})

add_subdirectory(lib/tinyobjloader-${TINYOBJLOADER_VERSION})

# Find Vulkan library and compile shaders
IF(WIN32)
  find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)

  execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/src/script/compile_shaders.bat ${SHADER_DIR} ${SHADER_COMPILER} ${GLSL_VERSION} ${NUM_BUFFERS} ${NUM_INSTANCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

ELSE(WIN32)
  find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
  
  execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/src/script/compile_shaders.sh ${SHADER_DIR} ${SHADER_COMPILER} ${GLSL_VERSION} ${NUM_BUFFERS} ${NUM_INSTANCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
ENDIF(WIN32)

set (LIBRARIES util
               allocator)

FOREACH(TARGET ${LIBRARIES})
  add_library(${TARGET} ${CPP_SOURCE_DIR}/${TARGET}.cpp)
ENDFOREACH(TARGET)

set (EXECUTABLES compute
		 graphics)

FOREACH(TARGET ${EXECUTABLES})
  IF(WIN32)
    IF(NOT ${TARGET} STREQUAL compute)
      add_executable(${TARGET} WIN32 ${CPP_SOURCE_DIR}/${TARGET}.cpp)
    ELSE(NOT ${TARGET} STREQUAL compute)
      add_executable(${TARGET} ${CPP_SOURCE_DIR}/${TARGET}.cpp)
    ENDIF(NOT ${TARGET} STREQUAL compute)
  ELSE(WIN32)
    add_executable(${TARGET} ${CPP_SOURCE_DIR}/${TARGET}.cpp)
    
    # Link Xlib
    find_package(X11 REQUIRED)
    target_link_libraries(${TARGET} ${X11_LIBRARIES})
    include_directories(${X11_INCLUDE_DIR})

    # Link XCB
    find_package(XCB REQUIRED)
    target_link_libraries(${TARGET} ${XCB_LIBRARIES})
  ENDIF(WIN32)

  # Link Vulkan
  target_link_libraries(${TARGET} ${Vulkan_LIBRARY})

  # Link threading libraries
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} ${CMAKE_THREAD_LIBS_INIT})

  FOREACH(LIB ${LIBRARIES})
    target_link_libraries(${TARGET} ${LIB})
  ENDFOREACH(LIB)
ENDFOREACH(TARGET)
