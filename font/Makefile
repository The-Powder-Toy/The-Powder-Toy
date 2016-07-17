
editor: editor.c
	gcc -oeditor -DSCALE=1 -DFONTEDITOR editor.c -lSDL -lm -O3 -ffast-math -march=k8
	
editor.exe: editor.c
	i686-w64-mingw32-gcc -oeditor.exe -DSCALE=1 -DFONTEDITOR editor.c -lmingw32 -lm -lSDLmain -lSDL -O3 -ffast-math -march=k8 -mwindows
	
packer: packer.c
	gcc -opacker -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8
	
packer.exe: packer.c
	i686-w64-mingw32-gcc -opacker.exe -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8
	
unpacker: unpacker.c
	gcc -ounpacker -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8

unpacker.exe: unpacker.c
	i686-w64-mingw32-gcc -ounpacker.exe -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8
	
clean:
	rm -f editor packer unpacker editor.exe packer.exe unpacker.exe
