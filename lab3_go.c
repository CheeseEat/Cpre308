#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* thread1();
void* thread2();

void main()
{
    pthread_t i1;
    pthread_t i2;

    pthread_create(&i1, NULL, &thread1, NULL);
    pthread_create(&i2, NULL, &thread2, NULL);
    pthread_join(i1, NULL);
    pthread_join(i2, NULL);
    printf("Hello im main function\n");
}

void* thread1()
{
    sleep(5);
    printf("Hello i am thread 1\n");
}

void* thread2()
{
    sleep(5);
    printf("hello i am thread 2\n");
}