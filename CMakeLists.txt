cmake_minimum_required(VERSION 3.20)
project(Corium)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(Corium SHARED 
	src/Public/Corium.h src/Private/Corium.cpp
	src/Public/CoriumMemory.h
	src/Public/CoriumUtils.h
	src/Public/CoriumEvent.h
	src/Public/Executors.h
	src/Public/ExecutorServices.h
	src/Public/CoriumFunctional.h	
	src/Public/TaskEngine.h
	src/Public/WorkStealUtils.h
	src/Public/CoriumTraits.h
	src/Public/ExecutorUtils.h
	src/Public/Memory.h
	src/Public/ThreadFactory.h
	src/Public/ThreadLocalStorage.h
	src/Public/ComposableServices.h
	src/Public/ThreadPlatform.h src/Private/ThreadPlatform.cpp
)

target_include_directories(Corium PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

set_target_properties(Corium PROPERTIES FOLDER "Base")

target_precompile_headers(Corium PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Corium.h)

# Compile options
add_compile_definitions(ENABLE_EVENT_EMITTERS_BLAZE = 0)