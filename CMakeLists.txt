    cmake_minimum_required(VERSION 3.20)
    project(Corium)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Enable folders in IDEs like Visual Studio
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    file(GLOB_RECURSE CORIUM_PUBLIC CONFIGURE_DEPENDS "src/Public/*.h")
    file(GLOB_RECURSE CORIUM_PRIVATE CONFIGURE_DEPENDS "src/Private/*.cpp")

    add_library(Corium SHARED ${CORIUM_PUBLIC} ${CORIUM_PRIVATE})

    target_include_directories(Corium PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    )

    set_target_properties(Corium PROPERTIES FOLDER "Base")

    target_precompile_headers(Corium PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Corium.h)

    # Compile options
target_compile_definitions(Corium PRIVATE
    CORIUM_SHARED
    CORIUM_VA_ALLOCATION=32756
)


    if (MSVC)
    set_property(TARGET Corium PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()