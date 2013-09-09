CC = gcc
TARGET = server client
CFLAGS = -c -Wall
LIBS = -lstdc++

all:	$(TARGET)

client: 
	$(CC) -o $@ $@.cpp $^ $(LIBS)

server: connection.o 
	$(CC) -o $@ $@.cpp $^ $(LIBS)

%.o:	%.c %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o 
	rm -f $(TARGET)
