./bin/libengine.a: binEngineFolder ./bin/Engine.o
	@echo [MAKE]: compressing libengine.a
	ar -vrs ./bin/libengine.a ./bin/engine/*.o ./bin/libglad.a ./Engine/externals/ubuntu/dependencies/*.a
	@echo [MAKE]: Done!
./bin/Engine.o: binEngineFolder binGladFolder ./Engine/EngineLib.cpp ./Engine/Engine.cpp GLFW ./bin/libglad.a
	@echo [MAKE]: Compiling Engine.o
	@cd ./bin/engine;\
	g++ -c -g \
	-I ../../Engine/Headers -I ../../Engine/externals/ubuntu/include -I /usr/include \
	../../Engine/*.cpp
GLFW:
# installs at /usr/lib/x86_64_linux-gnu/libglfw.so
# and headers at /usr/include/GLFW/glfw3.h
	@echo [MAKE]: installing GLFW
	@sudo apt-get update -y > /dev/null
	@sudo apt-get install libglfw3 -y > /dev/null
	@sudo apt-get install libglfw3-dev -y > /dev/null
	@echo [MAKE]: finished installing GLFW
./bin/libglad.a: binGladFolder ./bin/glad/glad.o
	@echo [MAKE]: compressing libglad.a
	@ar -rs ./bin/libglad.a ./bin/glad/glad.o > /dev/null
./bin/glad/glad.o: binGladFolder ./Engine/externals/dependencies/glad.c
	@echo [MAKE]: Compiling Glad
	@g++ -c -g -o ./bin/glad/glad.o -I Engine/externals/ubuntu/include Engine/externals/dependencies/glad.c

binEngineFolder: binFolder
	@-mkdir ./bin/engine
binGladFolder: binFolder
	@-mkdir ./bin/glad
binFolder:
	@-mkdir bin

clean:
	-rm -rf bin
	-rm -rf install

install:
	@echo [MAKE]: creating directories
	@-mkdir install
	@-mkdir install/libs
	@-mkdir install/dll
	@-mkdir install/includes
	@echo [MAKE]: copying libs
	@cp ./bin/libengine.a ./install/libs
	@echo [MAKE]: copying dlls
	@cp /usr/lib/x86_64-linux-gnu/libglfw.so ./install/dll
	@cp ./Engine/externals/ubuntu/dependencies/libportaudio.so.2.0.0 ./install/dll
	@echo [MAKE]: copying headers
	@-mkdir install/includes/Engine
	@-mkdir install/includes/glad
	@-mkdir install/includes/GLFW
	@-mkdir install/includes/KHR
	@-mkdir install/includes/PA
	@cp -a ./Engine/Headers/. ./install/includes/Engine
	@cp -a ./Engine/externals/ubuntu/include/glad/. ./install/includes/glad
	@cp -a /usr/include/GLFW/. ./install/includes/GLFW
	@cp -a ./Engine/externals/ubuntu/include/KHR/. ./install/includes/KHR
	@cp ./Engine/externals/ubuntu/include/pa_*.h ./install/includes/PA
	@cp ./Engine/externals/ubuntu/include/portaudio.h ./install/includes/PA
	@cp ./Engine/externals/ubuntu/include/sndfile.h ./install/includes
	@cp ./Engine/externals/ubuntu/include/sndfile.hh ./install/includes