CFLAGS = -O2 -fomit-frame-pointer -funroll-loops -Wall
LFLAGS = -lSDL -lSDL_mixer

all:
	g++ -o modest main.cpp $(CFLAGS) $(LFLAGS)

clean:
	rm -f *.o modest
