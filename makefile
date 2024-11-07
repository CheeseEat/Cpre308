edit: message.o lab1.o
	cc -o edit message.o lab1.o

message.o: message.c message.h
	cc -c message.c

lab1.o: lab1.c message.h
	cc -c lab1.c

