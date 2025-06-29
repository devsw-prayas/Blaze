cmake_minimum_required(VERSION 3.20)
project(Blaze)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(Blaze SHARED 
	src/Public/Blaze.h src/Private/Blaze.cpp
)

target_include_directories(Blaze PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

set_target_properties(Blaze PROPERTIES FOLDER "Base")
