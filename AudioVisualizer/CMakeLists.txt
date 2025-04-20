cmake_minimum_required(VERSION 3.12)
project(AudioVisualizer VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define SDL main handling - this is critical for Windows
add_definitions(-DSDL_MAIN_HANDLED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find required packages
find_package(OpenGL REQUIRED)

# Use vcpkg for SDL2
set(SDL2_DIR "C:/Users/Mikko/vcpkg/installed/x64-windows/share/sdl2")
find_package(SDL2 CONFIG REQUIRED)

find_package(GLEW REQUIRED)

# Optional: Find Lua for scripting
find_package(Lua QUIET)
if(NOT Lua_FOUND)
    # Try finding Lua with different names
    find_package(Lua54 QUIET)
    if(Lua54_FOUND)
        set(LUA_INCLUDE_DIR ${LUA54_INCLUDE_DIR})
        set(LUA_LIBRARIES ${LUA54_LIBRARIES})
        set(Lua_FOUND TRUE)
    else()
        find_package(Lua53 QUIET)
        if(Lua53_FOUND)
            set(LUA_INCLUDE_DIR ${LUA53_INCLUDE_DIR})
            set(LUA_LIBRARIES ${LUA53_LIBRARIES})
            set(Lua_FOUND TRUE)
        else()
            find_package(Lua51 QUIET)
            if(Lua51_FOUND)
                set(LUA_INCLUDE_DIR ${LUA51_INCLUDE_DIR})
                set(LUA_LIBRARIES ${LUA51_LIBRARIES})
                set(Lua_FOUND TRUE)
            endif()
        endif()
    endif()
endif()

if(Lua_FOUND)
    add_definitions(-DUSE_LUA)
    include_directories(${LUA_INCLUDE_DIR})
    message(STATUS "Found Lua: ${LUA_LIBRARIES}")
else()
    message(STATUS "Lua not found - scripting will be disabled")
endif()

# Source files
set(SOURCES
    # Core engine
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/InputManager.cpp
    
    # Audio processing
    src/audio/AudioProcessor.cpp
    src/audio/FFTAnalyzer.cpp
    
    # Rendering
    src/render/Renderer.cpp
    src/render/ParticleSystem.cpp
    src/render/ShaderManager.cpp
    
    # Visualizations
    src/visualizations/SimpleVisualizer.cpp
    src/visualizations/Visualization.cpp
    
    # Scripting
    src/scripting/ScriptEngine.cpp
    
    # Main entry point
    src/main.cpp
)

# Define main executable - use WIN32 for Windows GUI apps 
if(WIN32)
    add_executable(AudioVisualizer ${SOURCES})
    # SDL_MAIN_HANDLED is already defined globally
else()
    add_executable(AudioVisualizer ${SOURCES})
endif()

# Link libraries - use updated SDL2 targets without SDL2main
if(WIN32)
    target_link_libraries(AudioVisualizer
        ${OPENGL_LIBRARIES}
        SDL2::SDL2
        ${GLEW_LIBRARIES}
        ${LUA_LIBRARIES}
    )
else()
    target_link_libraries(AudioVisualizer
        ${OPENGL_LIBRARIES}
        SDL2::SDL2
        ${GLEW_LIBRARIES}
        ${LUA_LIBRARIES}
    )
endif()

# Installation
install(TARGETS AudioVisualizer DESTINATION bin)
install(DIRECTORY scripts/ DESTINATION bin/scripts)

# Add SDL test program as a separate executable
add_executable(test_sdl src/test_sdl.cpp)

# SDL_MAIN_HANDLED is already defined globally
# Link SDL test program with minimal dependencies
target_link_libraries(test_sdl SDL2::SDL2)

# Note: We're commenting out the custom SDL2 DLL copy since vcpkg handles this
# Copy necessary DLLs to output directory
if(WIN32)
    # No longer needed for vcpkg setups - it will handle the DLL copying
    #add_custom_command(TARGET AudioVisualizer POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        "${SDL2_LIBRARY_DIR}/SDL2.dll"
    #        $<TARGET_FILE_DIR:AudioVisualizer>
    #)
    
    add_custom_command(TARGET AudioVisualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_LIBRARY_DIR}/glew32.dll"
            $<TARGET_FILE_DIR:AudioVisualizer>
    )
    
    # No longer needed - vcpkg handles this
    #add_custom_command(TARGET test_sdl POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        "${SDL2_LIBRARY_DIR}/SDL2.dll"
    #        $<TARGET_FILE_DIR:test_sdl>
    #)
endif() 