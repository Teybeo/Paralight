cmake_minimum_required(VERSION 3.3)
project(Paralight)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_FLAGS -Wall -fopenmp)

set(BUILD_64_BIT "ON")
set(DEFINITIONS USE_OPENMP)
#list(APPEND DEFINITIONS USE_TRIGO_LOOKUP)
set(DEFINITIONS ${DEFINITIONS} __CL_ENABLE_EXCEPTIONS)

if (BUILD_64_BIT)
    list(APPEND COMMON_FLAGS -m64)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/64)
    message("Building as 64 Bit")
else()
    list(APPEND COMMON_FLAGS -m32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/32)
    message("Building as 32 Bit")
endif()

# Custom Debug definition to enable/disable stuff at compile-time
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_BUILD")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4 -mfpmath=sse")

if (NOT DEFINED ARCH)
    message("Builing for native arch")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
else (ARCH EQUAL generic)
    message("Builing for generic arch")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=sandybridge")
endif ()

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

############ OpenCL paths ############

# The find_library will use the system32/OpenCL.dll which will generates linker warnings
# This wasn't happening when linking to the NVIDIA dll but it has disapeared...

set(OPENCL_INCLUDE_DIR "D:/Dev/Tools/Nvidia CUDA Toolkit 7.5/include")
#set(OPENCL_LIB_DIR "D:/Dev/Tools/Nvidia CUDA Toolkit 7.5/lib/64")
#
#find_library(OPENCL_LIBRARY
#        NAMES OPENCL
#        HINTS ${OPENCL_LIB_DIR}
#        NO_DEFAULT_PATH
#        )

if (BUILD_64_BIT)
    set(OPENCL_LIBRARY "C:/Program Files/NVIDIA Corporation/OpenCL/OpenCL64.dll")
else()
    set(OPENCL_LIBRARY "C:/Program Files/NVIDIA Corporation/OpenCL/OpenCL.dll")
endif()

############ SDL paths ############

if (BUILD_64_BIT)
    set(SDL_INCLUDE_DIR "D:/Dev/Lib/SDL2-2.0.3/x86_64-w64-mingw32/include/SDL2")
    set(SDL_LIB_DIR     "D:/Dev/Lib/SDL2-2.0.3/x86_64-w64-mingw32/lib")
else()
    set(SDL_INCLUDE_DIR "D:/Dev/Lib/SDL2-2.0.3/i686-w64-mingw32/include/SDL2")
    set(SDL_LIB_DIR     "D:/Dev/Lib/SDL2-2.0.3/i686-w64-mingw32/lib")
endif()

find_library(SDL_LIBRARY
        NAMES SDL2
        HINTS ${SDL_LIB_DIR}
        NO_DEFAULT_PATH)

############ SDL_image paths ############

if (BUILD_64_BIT)
    set(SDL_IMG_INCLUDE_DIR "D:/Dev/Lib/SDL2_image-2.0.1/x86_64-w64-mingw32/include/SDL2")
    set(SDL_IMG_LIB_DIR     "D:/Dev/Lib/SDL2_image-2.0.1/x86_64-w64-mingw32/lib")
else()
    set(SDL_IMG_INCLUDE_DIR "D:/Dev/Lib/SDL2_image-2.0.1/i686-w64-mingw32/include/SDL2")
    set(SDL_IMG_LIB_DIR     "D:/Dev/Lib/SDL2_image-2.0.1/i686-w64-mingw32/lib")
endif()

find_library(SDL_IMG_LIBRARY
        NAMES SDL2_image
        HINTS ${SDL_IMG_LIB_DIR}
        NO_DEFAULT_PATH)

############ Assimp paths ############

set(ASSIMP_INCLUDE_DIR "D:/Dev/Lib/assimp-3.2/include")

if (BUILD_64_BIT)
    set(ASSIMP_LIB_DIR     "D:/Dev/Lib/assimp-3.2/lib/")
else()
    set(ASSIMP_LIB_DIR     "D:/Dev/Lib/assimp-3.2/build/code/")
endif()

find_library(ASSIMP_LIBRARY
        NAMES assimp
        HINTS ${ASSIMP_LIB_DIR}
        NO_DEFAULT_PATH)

#############################

set(LIBRARIES
        ${SDL_LIBRARY}
        ${SDL_IMG_LIBRARY}
        ${OPENCL_LIBRARY}
        ${ASSIMP_LIBRARY}
        opengl32
        imm32
        -fopenmp)

set(INCLUDE_DIRS
        src
        ${OPENCL_INCLUDE_DIR}
        ${SDL_INCLUDE_DIR}
        ${SDL_IMG_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIR})

include_directories(${INCLUDE_DIRS})

add_subdirectory(src)