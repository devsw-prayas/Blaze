cmake_minimum_required(VERSION 3.20)
project(Blaze)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Switch: Build Instrumenta internally or use externally
option(INSTRUMENTA_STANDALONE "Build Instrumenta internally for Blaze" ON)

if (INSTRUMENTA_STANDALONE)
    # Expect Instrumenta as a submodule under external/
    add_subdirectory(external/Instrumenta)
    include_directories(external/Instrumenta/src/Public)
    set(INSTRUMENTA_LIB Instrumenta)
else()
    set(INSTRUMENTA_LIB Instrumenta)
endif()

include_directories(src/Public)

add_library(Blaze SHARED
    src/Private/Blaze.cpp
    src/Public/Blaze.h
)

target_include_directories(Blaze PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_link_libraries(Blaze PRIVATE ${INSTRUMENTA_LIB})

set_target_properties(Blaze PROPERTIES FOLDER "Base")
