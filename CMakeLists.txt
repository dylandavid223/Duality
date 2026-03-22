cmake_minimum_required(VERSION 3.20)
project(DualityApocalypse VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ENABLE_RTX "Enable ray tracing support" ON)
option(ENABLE_DEBUG_MEMORY "Enable memory tracking" OFF)
option(ENABLE_VALIDATION_LAYERS "Enable Vulkan validation layers" OFF)

# Release build optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -flto")
    add_definitions(-DNDEBUG)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address")
    add_definitions(-DDUALITY_DEBUG)
endif()

# Windows specific
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

# Linux specific
if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Find Vulkan
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK.")
endif()

# Find GLFW
find_package(glfw3 REQUIRED)

# Find GLM
find_package(glm REQUIRED)

# Main executable
add_executable(DualityApocalypse
    # Core
    src/main.cpp
    src/Engine/Core/Logger.cpp
    src/Engine/Core/Memory.cpp
    src/Engine/Core/Engine.cpp
    src/Engine/Core/Window.cpp
    # Renderer
    src/Engine/Renderer/VulkanContext.cpp
    src/Engine/Renderer/RenderPass.cpp
    src/Engine/Renderer/Pipeline.cpp
    src/Engine/Renderer/ShaderCompiler.cpp
    src/Engine/Renderer/Mesh.cpp
    src/Engine/Renderer/Texture.cpp
    src/Engine/Renderer/Camera.cpp
    # Physics
    src/Engine/Physics/PhysicsWorld.cpp
    src/Engine/Physics/Collision.cpp
    # Audio
    src/Engine/Audio/AudioEngine.cpp
    # Input
    src/Engine/Input/InputSystem.cpp
    # Game
    src/Game/ApocalypseGame.cpp
    src/Game/World/WorldGenerator.cpp
    src/Game/World/Terrain.cpp
    src/Game/World/Chunk.cpp
    src/Game/Character/PlayerController.cpp
    src/Game/Character/ElsySystem.cpp
    src/Game/Character/NPC.cpp
    src/Game/Combat/WeaponSystem.cpp
    src/Game/UI/UIManager.cpp
)

# Include directories
target_include_directories(DualityApocalypse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${VULKAN_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(DualityApocalypse
    Vulkan::Vulkan
    glfw
    glm::glm
    pthread
    dl
)

# Windows specific libraries
if(WIN32)
    target_link_libraries(DualityApocalypse
        winmm
        ws2_32
        user32
        gdi32
        shell32
    )
endif()

# Linux specific
if(UNIX AND NOT APPLE)
    target_link_libraries(DualityApocalypse
        X11
        Xrandr
        Xi
        Xxf86vm
    )
endif()

# Shader compilation
find_program(GLSLC glslc HINTS ${VULKAN_SDK}/bin)

if(GLSLC)
    file(GLOB_RECURSE SHADERS shaders/*.vert shaders/*.frag shaders/*.comp)
    
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        get_filename_component(SHADER_EXT ${SHADER} EXT)
        
        if(SHADER_EXT STREQUAL ".vert")
            set(OUTPUT_EXT "vert.spv")
        elseif(SHADER_EXT STREQUAL ".frag")
            set(OUTPUT_EXT "frag.spv")
        elseif(SHADER_EXT STREQUAL ".comp")
            set(OUTPUT_EXT "comp.spv")
        endif()
        
        set(OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.${OUTPUT_EXT})
        
        add_custom_command(
            OUTPUT ${OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
            COMMAND ${GLSLC} ${SHADER} -o ${OUTPUT} --target-env=vulkan1.3 -O
            DEPENDS ${SHADER}
            COMMENT "Compiling shader: ${SHADER_NAME}"
        )
        
        list(APPEND SHADER_OUTPUTS ${OUTPUT})
    endforeach()
    
    add_custom_target(Shaders ALL DEPENDS ${SHADER_OUTPUTS})
    add_dependencies(DualityApocalypse Shaders)
endif()

# Copy assets
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets COPYONLY)

# Install
install(TARGETS DualityApocalypse RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_BINARY_DIR}/assets DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/shaders DESTINATION .)