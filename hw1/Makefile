CC = gcc
TARGET = server client
CFLAGS = -g  
LIBS = -lstdc++
DEP = connection.o sbcp.o

all:	$(TARGET)

client: client.cpp $(DEP) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

server: server.cpp $(DEP) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o:	%.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o 
	rm -f $(TARGET)
