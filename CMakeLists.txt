cmake_minimum_required(VERSION 3.20)
project(Blaze)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(Blaze SHARED 
	src/Public/Blaze.h src/Private/Blaze.cpp
	src/Public/BlazeMemory.h
	src/Public/BlazeUtils.h
	src/Public/BlazeEvent.h
	src/Public/Executors.h
	src/Public/ExecutorServices.h
	src/Public/BlazeFunctional.h	
	src/Public/TaskEngine.h
	src/Public/WorkStealUtils.h
	src/Public/BlazeTraits.h
	src/Public/ThreadPlatform.h src/Private/ThreadPlatform.cpp
)

target_include_directories(Blaze PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

set_target_properties(Blaze PROPERTIES FOLDER "Base")

target_precompile_headers(Blaze PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Blaze.h)

# Compile options
add_compile_definitions(ENABLE_EVENT_EMITTERS_BLAZE = 0)