cmake_minimum_required(VERSION 3.20)
project(Corium)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE CORIUM_PUBLIC "src/Public/*.h")
file(GLOB_RECURSE CORIUM_PRIVATE "src/Private/*.cpp")

add_library(Corium SHARED ${CORIUM_PUBLIC} ${CORIUM_PRIVATE})

target_include_directories(Corium PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

set_target_properties(Corium PROPERTIES FOLDER "Base")

target_precompile_headers(Corium PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Corium.h)

# Compile options
add_compile_definitions(ENABLE_EVENT_EMITTERS_CORIUM = 1)