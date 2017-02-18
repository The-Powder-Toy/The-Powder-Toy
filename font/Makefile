
editor: editor.c
	gcc -oeditor -DSCALE=1 -DFONTEDITOR editor.c -lSDL -lm -O3 -ffast-math -march=k8 -Wno-unused-result
	
editor.exe: editor.c
	i686-w64-mingw32-gcc -oeditor.exe -DSCALE=1 -DFONTEDITOR editor.c -lmingw32 -lm -lSDLmain -lSDL -O3 -ffast-math -march=k8 -mwindows -Wno-unused-result
	
packer: packer.c
	gcc -opacker -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8 -Wno-unused-result
	
packer.exe: packer.c
	i686-w64-mingw32-gcc -opacker.exe -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8 -Wno-unused-result
	
unpacker: unpacker.c
	gcc -ounpacker -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8 -Wno-unused-result

unpacker.exe: unpacker.c
	i686-w64-mingw32-gcc -ounpacker.exe -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8 -Wno-unused-result
	
clean:
	rm -f editor packer unpacker editor.exe packer.exe unpacker.exe
