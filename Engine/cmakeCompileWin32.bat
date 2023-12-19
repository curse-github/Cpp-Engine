@echo off
echo Building project
cmake -A WIN32 -G "Visual Studio 17 2022" -S . -B ./CMake
echo Copying depenencies directory
powershell -Command "xcopy .\imports .\CMake\imports\ /Eqy"
echo Copying resources into Src folder
powershell -Command "xcopy .\Data .\CMake\Data\ /Eqy"
powershell -Command "xcopy .\Fonts .\CMake\Fonts\ /Eqy"
powershell -Command "xcopy .\Resources .\CMake\Resources\ /Eqy"
echo Copying resources into Debug folder
powershell -Command "xcopy .\Data .\CMake\Debug\Data\ /Eqy"
powershell -Command "xcopy .\Fonts .\CMake\Debug\Fonts\ /Eqy"
powershell -Command "xcopy .\Resources .\CMake\Debug\Resources\ /Eqy"
echo Copying resources into Release folder
powershell -Command "xcopy .\Data .\CMake\Release\Data\ /Eqy"
powershell -Command "xcopy .\Fonts .\CMake\Release\Fonts\ /Eqy"
powershell -Command "xcopy .\Resources .\CMake\Release\Resources\ /Eqy"
cd ./CMake
cmake --build . --config Debug
cmake --build . --config Release
pause