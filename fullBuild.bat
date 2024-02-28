cmake -A Win32 -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -S . -B ./builds/Release32
cmake --build ./builds/Release32 --config Release
cmake --install ./builds/Release32 --config Release --prefix ./builds/Release32/BuildReleaseWin32
powershell -Command "Compress-Archive -Path ./builds/Release32/BuildReleaseWin32 -DestinationPath ./BuildReleaseWin32.zip" -Update

cmake -A x64 -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -S . -B ./builds/Release64
cmake --build ./builds/Release64 --config Release
cmake --install ./builds/Release64 --config Release --prefix ./builds/Release64/BuildReleaseWin64
powershell -Command "Compress-Archive -Path ./builds/Release64/BuildReleaseWin64 -DestinationPath ./BuildReleaseWin64.zip" -Update

cmake -A Win32 -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -S . -B ./builds/Debug32
cmake --build ./builds/Debug32 --config Debug
cmake --install ./builds/Debug32 --config Debug --prefix ./builds/Debug32/BuildDebugWin32
powershell -Command "Compress-Archive -Path ./builds/Debug32/BuildDebugWin32 -DestinationPath ./BuildDebugWin32.zip" -Update

cmake -A x64 -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -S . -B ./builds/Debug64
cmake --build ./builds/Debug64 --config Debug
cmake --install ./builds/Debug64 --config Debug --prefix ./builds/Debug64/BuildDebugWin64
powershell -Command "Compress-Archive -Path ./builds/Debug64/BuildDebugWin64 -DestinationPath ./BuildDebugWin64.zip" -Update
pause