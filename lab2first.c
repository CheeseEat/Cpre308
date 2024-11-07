#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// int main() {
//     int i=0, j=0, pid;
//     pid = fork();
//     if (pid == 0) {
//         for(i=0; i<500000; i++)
//             printf("Child: %d\n", i);
//         } 
//     else {
//         for(j=0; j<500000; j++)
//             printf("Parent: %d\n", j);
//     }
// }

// int main() {
//     int i=0, j=0, ret;
//     ret = fork();
//     if (ret == 0) {
//         printf("Child starts\n");
//         for(i=0; i<500000; i++)
//         {
//             printf("Child: %d\n", i);
//             printf("Child ends\n");
//         }
//     } else {
//         wait(NULL);
//         printf("Parent starts\n");
//         for(j=0; j<500000; j++)
//         {
//             printf("Parent: %d\n", j);
//             printf("Parent ends\n");
//         }
//     }        
//     return 0;
// }

// int main() {
//     int i=0, child;
//     child = fork();
//     if (child == 0) {
//         while (1) {
//             i++;
//             printf("Child at count %d\n", i);
//             /* sleep for 1/100 of a second */
//             usleep(10000);
//         }
//     } else {
//         printf("Parent sleeping\n");
//         sleep(10);
//         kill(child, SIGTERM);
//         printf("Child has been killed. Waiting for it...\n");
//         wait(NULL);
//         printf("done.\n");  
//     }
//     return 0;
// }

// void main() {
//     execl("/bin/ls", "ls", NULL);
//     printf("What happened?\n");
// }

int main(int argc, char** argv) {
    int child, status;
    child = fork();
    if (child == 0) {
        srand(time(NULL));
        if (rand() < RAND_MAX/4) {
            /* kill self with a signal */
            kill(getpid(), SIGTERM);
        }
        return rand();
    } else {
        wait(&status);
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child exited with signal %d\n", WTERMSIG(status));
        }
    }
    return 0;
}