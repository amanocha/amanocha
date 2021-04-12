TARGET=sim
CC=g++
CPPFLAGS=-std=c++11

all:
	$(CC) $(CPPFLAGS) -o sim $(TARGET).cc

clean:
	rm -f $(TARGET)
