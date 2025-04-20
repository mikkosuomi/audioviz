@echo off
setlocal

echo Creating build directory...
if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/Mikko/vcpkg/scripts/buildsystems/vcpkg.cmake ..
if %ERRORLEVEL% neq 0 (
  echo CMake configuration failed.
  exit /b 1
)

echo Building with CMake...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
  echo Build failed.
  exit /b 1
)

echo.
echo Build completed successfully!
echo The executable should be in build\bin\Release\AudioVisualizer.exe
echo.

cd .. 