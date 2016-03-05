all:
	gcc -Wall pendulum.c -o pendulum `pkg-config --cflags --libs gtk+-3.0` -lm
	gcc -Wall backend.c -o backend `pkg-config --cflags --libs gsl`

clean:
		rm -f *.o
