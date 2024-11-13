CC = cc
CFLAGS = -Wall -g -pthread

#objects = bankserver.o Project2Test_v2.o Bank.o
objects = appserver.o Bank.o

appserver: $(objects)
	$(CC) -o bankapp $(objects) -pthread

appserver.o: appserver.c Bank.h
	$(CC) -c appserver.c $(CFLAGS)

# Project2Test_v2.o: Project2Test_v2.c
# 	$(CC) -c Project2Test_v2.c $(CFLAGS)

Bank.o: Bank.c Bank.h
	$(CC) -c Bank.c $(CFLAGS)

clean:
	rm -f bankapp $(objects)
