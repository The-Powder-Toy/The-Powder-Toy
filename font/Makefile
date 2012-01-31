editor: editor.c
	gcc -oeditor -DSCALE=2 -DFONTEDITOR editor.c -lSDL -lm -O3 -ffast-math -march=k8 -Wall -std=c99
	
editor.exe: editor.c
	gcc -oeditor.exe -DSCALE=2 -DFONTEDITOR editor.c -lmingw32 -lm -lSDLmain -lSDL -O3 -ffast-math -march=k8 -Wall -std=c99 -mwindows
	
packer: packer.c
	gcc -opacker -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8 -Wall -std=c99
	
packer.exe: packer.c
	gcc -opacker.exe -DFONTEDITOR packer.c -lm -O3 -ffast-math -march=k8 -Wall -std=c99
	
unpacker: unpacker.c
	gcc -ounpacker -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8 -Wall -std=c99

unpacker.exe: unpacker.c
	gcc -ounpacker.exe -DFONTEDITOR unpacker.c -lm -O3 -ffast-math -march=k8 -Wall -std=c99
	
clean:
	rm -f editor packer unpacker editor.exe packer.exe unpacker.exe
