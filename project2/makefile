CC = cc
CFLAGS = -Wall -g -pthread

#objects = bankserver.o Project2Test_v2.o Bank.o
objects = bankserver.o Bank.o

bankapp: $(objects)
	$(CC) -o bankapp $(objects) -pthread

bankserver.o: bankserver.c Bank.h
	$(CC) -c bankserver.c $(CFLAGS)

# Project2Test_v2.o: Project2Test_v2.c
# 	$(CC) -c Project2Test_v2.c $(CFLAGS)

Bank.o: Bank.c Bank.h
	$(CC) -c Bank.c $(CFLAGS)

clean:
	rm -f bankapp $(objects)
