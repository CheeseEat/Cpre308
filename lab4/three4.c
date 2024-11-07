#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char msg[100];
void my_alarm();
int main(int argc, char * argv[]){
    int time;
    if (argc < 3) {
        printf("not enough parameters\n");
        exit(1);
    }
    time = atoi(argv[2]);   // Converts third argument into int time
    strcpy(msg, argv[1]);   // Copying second argument into message, which will get printed by the alarm
    signal(SIGALRM, my_alarm);  // Alarm gets triggered by SIGALRM signal, SIGALARM gets triggered when the time for a specified time runs out
    alarm(time);                // This is specifying the time to wait before raising the signal, which is our third arg into program
    printf("Entering infinite loop\n");
    while (1) 
    { 
        sleep(10); 
    }
    printf("Can’t get here\n");
}
void my_alarm() {
    printf("%s\n", msg);
    exit(0);
}