#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void my_routine();

int main()
{

    signal(SIGFPE, my_routine);

    int a = 4;
    a = a/0;

    return 0;

}

void my_routine() {
    printf("“Caught a SIGFPE\n");
}