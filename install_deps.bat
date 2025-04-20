@echo off
setlocal enabledelayedexpansion

echo Installing dependencies for AudioVisualizer...

:: Check if vcpkg directory exists
if not exist vcpkg (
    echo Downloading vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call bootstrap-vcpkg.bat
    cd ..
) else (
    echo vcpkg directory already exists.
)

:: Install dependencies
echo Installing dependencies with vcpkg...
call vcpkg\vcpkg install sdl2:x64-windows sdl2-image:x64-windows glew:x64-windows glm:x64-windows

echo Dependencies installed successfully!
endlocal 