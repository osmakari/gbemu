gbemu: src/*.c src/*.h src/x11-window/*.c src/x11-window/*.h
	gcc src/*.c src/x11-window/*.c -lX11 -o bin/gbemu