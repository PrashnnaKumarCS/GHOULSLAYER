cmake_minimum_required(VERSION 3.21)

project(GHOULSLAYER
    VERSION 0.1.0
    LANGUAGES CXX
)

# Standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SFML Configuration (Static build, exclude unused modules)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Build network library" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "Build documentation" FORCE)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.1
    GIT_SHALLOW    ON
)

FetchContent_MakeAvailable(SFML)

# Collect sources and headers
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "src/*.hpp")

add_executable(GHOULSLAYER
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(GHOULSLAYER PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# SFML static linkage definition
target_compile_definitions(GHOULSLAYER PRIVATE SFML_STATIC)

# Link SFML components statically
target_link_libraries(GHOULSLAYER PRIVATE
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
)

# Static link MinGW runtime for standalone Windows execution
if(MINGW)
    target_link_options(GHOULSLAYER PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()
