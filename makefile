gbemu: src/*.c src/*.h src/x11-window/*.c src/x11-window/*.h
	gcc src/*.c src/x11-window/*.c -lpthread -lX11 -o bin/gbemu