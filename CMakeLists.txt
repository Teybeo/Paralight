cmake_minimum_required(VERSION 3.3)
project(Paralight)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_OPENMP "Use OpenMP to parallelize the CPU renderer")

if (USE_OPENMP)
    list(APPEND COMMON_FLAGS -fopenmp)
    set(OPENMP_LIBRARY -fopenmp)
    set(DEFINITIONS USE_OPENMP)
endif()

#list(APPEND DEFINITIONS USE_TRIGO_LOOKUP)
set(DEFINITIONS ${DEFINITIONS} __CL_ENABLE_EXCEPTIONS)

list(APPEND COMMON_FLAGS -m64)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/64)
message("Building as 64 Bit")

# Custom Debug macro to enable/disable stuff at compile-time
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_BUILD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4 -mfpmath=sse")

message("---- Compiler Flags ----")
foreach(flag ${COMMON_FLAGS})
    message("${flag}")
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(${CMAKE_CXX_FLAGS_DEBUG})
else (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(${CMAKE_CXX_FLAGS_RELEASE})
endif ()

message("---- Compiler Defines ----")
foreach(define ${DEFINITIONS})
    message("${define}")
endforeach()

# GS60: haswell
# Desktop: bdver1

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -ffast-math")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -msse4 -mfpmath=sse")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -mtune=native -march=native")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-default-inline")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g3")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg") #Causes a freeze with OpenMP

############ OpenCL (library) paths ############

# The find_library will use the system32/OpenCL.dll which will generates linker warnings
# This wasn't happening when linking to the NVIDIA dll but it has disapeared...

set(OPENCL_LIBRARY "/System/Library/Frameworks/OpenCL.framework/OpenCL")
#set(OPENCL_LIBRARY "C:/Program Files/NVIDIA Corporation/OpenCL/OpenCL.dll")

#set(OPENCL_INCLUDE_DIR "D:/Dev/Tools/Nvidia CUDA Toolkit 7.5/include")
#set(OPENCL_LIB_DIR "D:/Dev/Tools/Nvidia CUDA Toolkit 7.5/lib/64")
#
#find_library(OPENCL_LIBRARY
#        NAMES OPENCL
#        HINTS ${OPENCL_LIB_DIR}
#        NO_DEFAULT_PATH
#        )

############ OpenCL C++ Bindings header paths ############

set(OPENCL_INCLUDE_DIR "$ENV{HOME}/.brew/include/")

############ SDL paths ############

set(SDL_BASE_DIR "$ENV{HOME}/.brew/Cellar/sdl2/2.0.8")
#set(SDL_BASE_DIR "D:/Dev/Lib/SDL2-2.0.3/x86_64-w64-mingw32/")

set(SDL_INCLUDE_DIR "${SDL_BASE_DIR}/include/SDL2")
set(SDL_LIB_DIR     "${SDL_BASE_DIR}/lib")

find_library(SDL_LIBRARY
        NAMES SDL2
        HINTS ${SDL_LIB_DIR}
        NO_DEFAULT_PATH)

############ SDL_image paths ############

set(SDL_IMG_BASE_DIR "$ENV{HOME}/.brew/Cellar/sdl2_image/2.0.3")
#set(SDL_IMG_BASE_DIR "D:/Dev/Lib/SDL2_image-2.0.1/x86_64-w64-mingw32")

set(SDL_IMG_INCLUDE_DIR "${SDL_IMG_BASE_DIR}/include/SDL2")
set(SDL_IMG_LIB_DIR     "${SDL_IMG_BASE_DIR}/lib")

find_library(SDL_IMG_LIBRARY
        NAMES SDL2_image
        HINTS ${SDL_IMG_LIB_DIR}
        NO_DEFAULT_PATH)

############ Assimp paths ############

set(ASSIMP_BASE_DIR "$ENV{HOME}/.brew/Cellar/assimp/4.1.0")
#set(ASSIMP_BASE_DIR "D:/Dev/Lib/assimp-4.1.0")

set(ASSIMP_INCLUDE_DIR "${ASSIMP_BASE_DIR}/include/")
set(ASSIMP_LIB_DIR     "${ASSIMP_BASE_DIR}/lib/")

find_library(ASSIMP_LIBRARY
        NAMES assimp
        HINTS ${ASSIMP_LIB_DIR}
        NO_DEFAULT_PATH)

############ OpenGL ############

find_package(OpenGL REQUIRED)

#############################

set(LIBRARIES
        ${SDL_LIBRARY}
        ${SDL_IMG_LIBRARY}
        ${ASSIMP_LIBRARY}
#        opengl32
#        imm32
        ${OPENGL_gl_LIBRARY}
        ${OPENCL_LIBRARY}
        ${OPENMP_LIBRARY})

set(INCLUDE_DIRS
        src
        ${SDL_INCLUDE_DIR}
        ${SDL_IMG_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIR}
        ${OPENCL_INCLUDE_DIR})

include_directories(${INCLUDE_DIRS})

add_subdirectory(src)