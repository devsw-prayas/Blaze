cmake_minimum_required(VERSION 3.20)

option(CORIUM_ENABLE_CUDA "Enable CUDA GPU backend" ON)

if (CORIUM_ENABLE_CUDA)
    project(Corium LANGUAGES CXX CUDA)
    set(CMAKE_CUDA_ARCHITECTURES 80 86) # Supports only Ampere for now
else()
    project(Corium LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE CORIUM_PUBLIC    CONFIGURE_DEPENDS "src/Public/*.h")
file(GLOB_RECURSE CORIUM_PRIVATE   CONFIGURE_DEPENDS "src/Private/*.cpp")
file(GLOB_RECURSE CORIUM_INTERNAL  CONFIGURE_DEPENDS "src/Internal/*.h")

if (CORIUM_ENABLE_CUDA)
    file(GLOB_RECURSE CORIUM_CUDA CONFIGURE_DEPENDS
        "src/Cuda/*.cu"
        "src/Cuda/*.cuh"
        "src/Cuda/*.cpp"
        "src/Cuda/*.h"
    )
endif()

add_library(Corium SHARED
    ${CORIUM_PUBLIC}
    ${CORIUM_PRIVATE}
    ${CORIUM_INTERNAL}
    $<$<BOOL:${CORIUM_ENABLE_CUDA}>:${CORIUM_CUDA}>
)

# Listed unconditionally (not through the $<BOOL:...> guard above) -
# source_group only registers a virtual-folder mapping, it doesn't need
# the generator-expression gate add_library uses to conditionally compile
# these files.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${CORIUM_PUBLIC}
    ${CORIUM_PRIVATE}
    ${CORIUM_INTERNAL}
    ${CORIUM_CUDA}
)

target_include_directories(Corium PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_include_directories(Corium PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

if (CORIUM_ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(Corium PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Cuda
    )
    target_link_libraries(Corium PRIVATE CUDA::cuda_driver)
    target_compile_definitions(Corium PRIVATE CORIUM_CUDA_AVAILABLE)
    set_target_properties(Corium PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_STANDARD 20
    )
endif()

set_target_properties(Corium PROPERTIES FOLDER "Base")

target_precompile_headers(Corium PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Corium.h
)

target_compile_definitions(Corium PRIVATE  CORIUM_SHARED)
target_compile_definitions(Corium PRIVATE  CORIUM_MAX_NUMA=4)
target_compile_definitions(Corium PRIVATE  CORIUM_VA_ALLOCATION=524288)
target_compile_definitions(Corium PRIVATE  CORIUM_TIMEOUT=50)
target_compile_definitions(Corium PRIVATE  CORIUM_SPIN_COUNT=450)
target_compile_definitions(Corium PRIVATE  CORIUM_DEFAULT_OS_TLS_SIZE=2)

target_compile_definitions(Corium PUBLIC  CORIUM_SPACE_TLS_MIN_SIZE=2)
target_compile_definitions(Corium PUBLIC  CORIUM_SPACE_TLS_MAX_SIZE=100)

option(CORIUM_ENABLE_DEBUG_CHECKS "Enable Corium debug asserts/checks" ON)
target_compile_definitions(Corium PUBLIC $<$<BOOL:${CORIUM_ENABLE_DEBUG_CHECKS}>:DEBUG>)

if (MSVC)
    set_property(TARGET Corium PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()