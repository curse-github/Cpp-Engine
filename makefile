Library: buildFolder Compile GLFW
	@echo [Engine]: Compressing libengine.a
	@ar -rs ./builds/linux/libengine.a ./builds/linux/engine/*.o
	@echo [Engine]: De-compiling freetype
	@cd ./builds/linux/freetype;ar -x ../../../Engine/externals/ubuntu/dependencies/libfreetype.a
	@echo [Engine]: De-compiling portaudio
	@cd ./builds/linux/portaudio;ar -x ../../../Engine/externals/ubuntu/dependencies/libportaudio.a
	@echo [Engine]: De-compiling sndfile
	@cd ./builds/linux/sndfile;ar -x ../../../Engine/externals/ubuntu/dependencies/libsndfile.a
	@echo [Engine]: Compressing libengineCombined.a
	@ar -rs ./builds/linux/libengineCombined.a ./builds/linux/engine/*.o ./builds/linux/freetype/*.o ./builds/linux/glad/*.o ./builds/linux/portaudio/*.o ./builds/linux/sndfile/*.o
	@echo [Engine]: Done!
Compile: buildFolder GLFW ./builds/linux/libglad.a
	@echo [Engine]: Compiling .cpp files
	@cd ./builds/linux/engine;\
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
./builds/linux/libglad.a: buildFolder
	@echo [Engine]: Compiling Glad
	@g++ -c -g -o ./builds/linux/glad/glad.o -I Engine/externals/ubuntu/include Engine/externals/dependencies/glad.c
	@echo [Engine]: Compressing libglad.a
	@ar -rs ./builds/linux/libglad.a ./builds/linux/glad/glad.o > /dev/null

buildFolder:
	@-mkdir builds
	@-mkdir ./builds/linux
	@-mkdir ./builds/linux/glad
	@-mkdir ./builds/linux/engine
	@-mkdir ./builds/linux/freetype
	@-mkdir ./builds/linux/portaudio
	@-mkdir ./builds/linux/sndfile
	@-mkdir ./builds/linux/bin
	@-mkdir ./builds/linux/bin/Data
	@-mkdir ./builds/linux/bin/Resources

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
	@cp ./builds/linux/*.a ./install/libs
	@cp ./Engine/externals/ubuntu/dependencies/*.a ./install/libs
	@echo [Engine]: Copying dlls
	@cp /usr/lib/x86_64-linux-gnu/libglfw.so ./install/dll
	@cp ./Engine/externals/ubuntu/dependencies/libportaudio.so.2.0.0 ./install/dll
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
