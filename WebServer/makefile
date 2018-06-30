CC = gcc # Compiler

myhttpd: server.o thread.o Queue.o functions.o threadpool.o
	$(CC) -o myhttpd server.o thread.o Queue.o functions.o threadpool.o -lpthread -lm

server.o: server.c
	$(CC) -c server.c

thread.o: thread.c
	$(CC) -c thread.c

Queue.o: Queue.c
	$(CC) -c Queue.c

functions.o: functions.c
	$(CC) -c functions.c

threadpool.o: threadpool.c
	$(CC) -c threadpool.c

.Phony: clean

clean:
	rm -r myhttpd server.o thread.o Queue.o functions.o threadpool.o
