CC = gcc
TARGET = server client
CFLAGS = -c -Wall
LIBS = 

all:	$(TARGET)

client: 
	$(CC) -o $@ $@.c $^ $(LIBS)

server: 
	$(CC) -o $@ $@.c $^ $(LIBS)

%.o:	%.c %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o 
	rm -f $(TARGET)
