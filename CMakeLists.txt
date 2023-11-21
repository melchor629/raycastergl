cmake_minimum_required(VERSION 3.15)
project(raycastergl)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

option(BUILD_TESTING FALSE)
option(YAML_CPP_BUILD_CONTRIB FALSE)
option(YAML_CPP_BUILD_TESTS FALSE)
option(YAML_CPP_BUILD_TOOLS FALSE)
option(YAML_CPP_INSTALL FALSE)

FetchContent_Declare(
    argumentum
    GIT_REPOSITORY  https://github.com/mmahnic/argumentum.git
    GIT_TAG         v0.3.2
    GIT_PROGRESS    TRUE
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         0.9.9.8
    GIT_PROGRESS    TRUE
)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp.git
    GIT_TAG         0.8.0
    GIT_PROGRESS    TRUE
)
FetchContent_Declare(
    stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         03f50e343d796e492e6579a11143a085429d7f5d
    GIT_PROGRESS    TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND   ""
)
FetchContent_MakeAvailable(argumentum glm yaml-cpp stb)

add_subdirectory(vendor/glad)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

find_package(glfw3 REQUIRED)

# Sanitizers (linux only)
set(ADDRESS_SANITIZE FALSE CACHE BOOL "Enables Address Sanitizer")
set(UNDEFINED_SANITIZE FALSE CACHE BOOL "Enables Undefined Behaviour Sanitizer")
if(ADDRESS_SANITIZE)
    message("Enabled Address Sanitizer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()
if(UNDEFINED_SANITIZE)
    message("Enabled Undefined Sanitizer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined")
endif()

set(RAYCASTERGL_HEADERS
    raycastergl/headers/arguments.hpp
    raycastergl/headers/opengl/texture.hpp
    raycastergl/headers/opengl/shader-program.hpp
    raycastergl/headers/opengl/check-error.hpp
    raycastergl/headers/opengl/buffer-geometry.hpp
    raycastergl/headers/opengl/shader.hpp
    raycastergl/headers/opengl/buffer.hpp
    raycastergl/headers/opengl/buffer-attribute.hpp
    raycastergl/headers/engine/map.hpp
    raycastergl/headers/engine/sprite.hpp
    raycastergl/headers/utils/files.hpp
    raycastergl/headers/utils/defer.hpp
)
set(RAYCASTERGL_SOURCES
    raycastergl/src/main.cpp
    raycastergl/src/opengl/shader-program.cpp
    raycastergl/src/opengl/shader.cpp
    raycastergl/src/opengl/buffer.cpp
    raycastergl/src/opengl/check-error.cpp
    raycastergl/src/opengl/texture.cpp
    raycastergl/src/opengl/buffer-geometry.cpp
    raycastergl/src/arguments.cpp
    raycastergl/src/engine/map.cpp
    raycastergl/src/utils/files.cpp
    raycastergl/src/utils/stb.c
)
set(RAYCASTERGL_SHADERS
    raycastergl/res/shaders/vert.glsl
    raycastergl/res/shaders/raycaster.glsl
    raycastergl/res/shaders/raycaster-drawer.glsl
    raycastergl/res/shaders/spritecaster.glsl
)
include_directories(raycastergl/headers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/download.cmake 
  "file(DOWNLOAD https://lodev.org/cgtutor/files/wolftex.zip ${CMAKE_CURRENT_BINARY_DIR}/wolftex.zip)"
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/extract.cmake 
  "file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/wolftex.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR})"
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pics/eagle.png
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/download.cmake
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/extract.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  VERBATIM
)
add_custom_target(textures DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pics/eagle.png)

set_target_properties(textures
  PROPERTIES
  ADDITIONAL_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/pics
)

add_executable(raycastergl
    ${RAYCASTERGL_HEADERS}
    ${RAYCASTERGL_SOURCES}
    ${RAYCASTERGL_SHADERS}
)
add_dependencies(raycastergl textures)

target_link_libraries(raycastergl
    GLAD
    yaml-cpp
    glm::glm
    glfw
    stb
    Argumentum::headers
)

add_custom_target(
    copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/raycastergl/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/pics $<TARGET_FILE_DIR:${PROJECT_NAME}>/res/textures
)
