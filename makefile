Library: buildFolder Compile GLFW
	@echo [Engine]: Compressing libengine.a
	@ar -rs ./builds/ubuntu/libengine.a ./builds/ubuntu/engine/*.o
	@echo [Engine]: Done!
Compile: buildFolder GLFW ./builds/ubuntu/libglad.a
	@echo [Engine]: Compiling .cpp files
	@cd ./builds/ubuntu/engine;\
	g++ -c -g \
	-I ../../../Engine/Headers -I ../../../Engine/externals/ubuntu/include -I /usr/include \
	../../../Engine/*.cpp
GLFW:
# installs at /usr/lib/x86_64_linux-gnu/libglfw.so
# and headers at /usr/include/GLFW/glfw3.h
	@echo [Engine]: Installing GLFW
	@sudo apt-get update -y > /dev/null
	@sudo apt-get install libglfw3 -y > /dev/null
	@sudo apt-get install libglfw3-dev -y > /dev/null
	@echo [Engine]: Finished installing GLFW
./builds/ubuntu/libglad.a: buildFolder
	@echo [Engine]: Compiling Glad
	@g++ -c -g -o ./builds/ubuntu/glad/glad.o -I Engine/externals/ubuntu/include Engine/externals/dependencies/glad.c
	@echo [Engine]: Compressing libglad.a
	@ar -rs ./builds/ubuntu/libglad.a ./builds/ubuntu/glad/glad.o > /dev/null

buildFolder:
	@echo [Engine]: Creating build directories
	@-mkdir builds
	@-mkdir ./builds/ubuntu
	@-mkdir ./builds/ubuntu/engine
	@-mkdir ./builds/ubuntu/glad
	@-mkdir ./builds/ubuntu/bin
	@-mkdir ./builds/ubuntu/bin/Data
	@-mkdir ./builds/ubuntu/bin/Resources

clean:
	-rm -rf builds
	-rm -rf install

install:
	@echo [Engine]: Creating install directories
	@-mkdir install
	@-mkdir install/libs
	@-mkdir install/dll
	@-mkdir install/includes
	@echo [Engine]: Copying libs
	@cp ./builds/ubuntu/*.a ./install/libs
	@cp ./Engine/externals/ubuntu/dependencies/*.a ./install/libs
	@echo [Engine]: Copying dlls
	@cp /usr/lib/x86_64-linux-gnu/libglfw.so ./install/dll
	@cp ./Engine/externals/ubuntu/dependencies/*.so ./install/dll
	@cp ./Engine/externals/ubuntu/dependencies/*.so.* ./install/dll
	@echo [Engine]: Copying headers
	@-mkdir install/includes/Engine
	@-mkdir install/includes/glad
	@-mkdir install/includes/GLFW
	@-mkdir install/includes/KHR
	@cp -a ./Engine/Headers/. ./install/includes/Engine
	@cp -a ./Engine/externals/ubuntu/include/glad/. ./install/includes/glad
	@cp -a /usr/include/GLFW/. ./install/includes/GLFW
	@cp -a ./Engine/externals/ubuntu/include/KHR/. ./install/includes/KHR
	@cp ./Engine/externals/ubuntu/include/pa_*.h ./install/includes
	@cp ./Engine/externals/ubuntu/include/portaudio.h ./install/includes
	@cp ./Engine/externals/ubuntu/include/sndfile.h ./install/includes
	@cp ./Engine/externals/ubuntu/include/sndfile.hh ./install/includes
