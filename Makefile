prodcon: prodcon.o tands.o
	# using -O instead of -g
	gcc -o prodcon prodcon.o tands.o -O -lpthread

prodcon.o: main.c tands.h
	gcc -o prodcon.o -c main.c

tands.o: tands.c tands.h
	gcc -c tands.c

clean:
	rm prodcon prodcon.o tands.o *.log