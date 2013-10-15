Homework 3 Submission
ECEN 602 : Computer Communication and Networks

By:
Suvodeep Pyne
Yang Song

Usage Syntax:
	./proxyserver <ip to bind> <port to bind> 
	./httpclient <proxy address> <proxy port> <url to retrieve> 

  example
	./proxyserver localhost 8080
	./httpclient localhost 8080 http://165.91.215.188/index.html


Proxy Server Architecture

The lru.h and lru.cpp files contains the implementation for LRU and its
associated structures. The socket code is present in proxyserver.cpp.
Also most of the logic for interacting with the host or the cache is in
the same file.

Client Architecture

The client code is in httpclient.cpp. It contains the socket code and 
also the functions for creating an HTTP GET packet.


