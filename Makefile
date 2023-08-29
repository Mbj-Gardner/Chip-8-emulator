build:
	gcc -Wall -std=c99 -g *.c -o Emulator -I include -L lib -l SDL2-2.0.0
run:
	./Emulator
clean:
	rm Emulator
