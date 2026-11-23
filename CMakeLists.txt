cmake_minimum_required(VERSION 3.21)

project(GHOULSLAYER
    VERSION 0.3.0
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

# Explicitly list source and header files.
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Map.cpp
    src/Camera.cpp
    src/ResourceManager.cpp
    src/PlaceholderArt.cpp
    src/MinGWCompat.cpp
    src/Bullet.cpp
    src/Weapon.cpp
    src/Zombie.cpp
    src/CollisionSystem.cpp
    src/BloodSystem.cpp
)

set(HEADERS
    src/Config.hpp
    include/Game.hpp
    include/Player.hpp
    include/Map.hpp
    include/Camera.hpp
    include/ResourceManager.hpp
    include/PlaceholderArt.hpp
    include/Bullet.hpp
    include/Weapon.hpp
    include/Zombie.hpp
    include/CollisionSystem.hpp
    include/BloodSystem.hpp
)

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

# Automatically copy OpenAL DLL next to executable for audio support
if(WIN32 AND EXISTS "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
    add_custom_command(TARGET GHOULSLAYER POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${sfml_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
            "$<TARGET_FILE_DIR:GHOULSLAYER>"
        VERBATIM
    )
endif()
