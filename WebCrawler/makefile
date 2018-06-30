CC = gcc # Compiler

mycrawler: client.o functions.o Queue.o socket.o thread.o threadpool.o
	
	$(CC) -o mycrawler client.o functions.o Queue.o socket.o thread.o threadpool.o -lpthread -lm

client.o: client.c
	$(CC) -c client.c

functions.o: functions.c
	$(CC) -c functions.c

Queue.o: Queue.c
	$(CC) -c Queue.c

socket.o: socket.c
	$(CC) -c socket.c

thread.o: thread.c
	$(CC) -c thread.c

threadpool.o: threadpool.c
	$(CC) -c threadpool.c

.Phony: clean

clean:
	rm -r mycrawler client.o functions.o Queue.o socket.o thread.o threadpool.o
