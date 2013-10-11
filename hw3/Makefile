CC = gcc
TARGET = proxyserver httpclient
CFLAGS = -g  
LIBS = -lstdc++
DEP =

all:    $(TARGET)

httpclient: httpclient.cpp $(DEP) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

proxyserver: proxyserver.cpp $(DEP) 
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o:    %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o 
	rm -f $(TARGET)

