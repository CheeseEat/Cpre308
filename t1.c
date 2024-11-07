/* t1.c 
   use mutex to access critical area 
   compile use: gcc -o t1 t1.c -lpthread 
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* define two routines called by threads*/
void 	increment();	// increment global variable		    
void	decrement();	// decrement global variable


/* global variable: mutex*/
pthread_mutex_t mutex;  				

/*global variable: counter*/
int v;

struct trans { // structure for a transaction pair
    int acc_id; // account ID
    int amount; // amount to be added, could be positive or negative
};
struct request {
    struct request * next; // pointer to the next request in the list
    int request_id; // request ID assigned by the main thread
    int check_acc_id; // account ID for a CHECK request
    struct trans * transactions; // array of transaction data
    int num_trans; // number of accounts in this transaction
    struct timeval starttime, endtime; // starttime and endtime for TIME
};
struct queue {
    struct request * head, * tail; // head and tail of the list
    int num_jobs; // number of jobs currently in queue
};


int main (int argc, char *argv[])
{
  /* thread id or type*/
  pthread_t tid_increment;			
  pthread_t	tid_decrement; 
  
  /*initialize v as 0*/
  v = 0;
  
  /* mutex initialization*/
  pthread_mutex_init(&mutex, NULL);	
  
  /* thread creation */
  pthread_create(&tid_increment, NULL, (void*)&increment, NULL); 
  pthread_create(&tid_decrement, NULL, (void*)&decrement, NULL); 
  
  /* main waits for the two threads to finish */
  pthread_join(tid_increment, NULL);  
  pthread_join(tid_decrement, NULL);
  
  printf("v=%d\n",v);
  
  return 0;
}



void increment()
{   
  int	i;
  int a;
  for( i = 0; i < 99; i++) 
  {	
    //pthread_mutex_lock(&mutex);
    
    /* critical section */
    a = v + 10;
    usleep(1);
    v = a;
    /* end of critical section */
    
    //pthread_mutex_unlock(&mutex);
  }
}


void decrement()
{	
  int  i;
  int  a;
  
  for(i = 0; i < 99; i++)
  {	
    //pthread_mutex_lock(&mutex);
    
    /* critical section */
    a = v - 10;
    usleep(2);
    v = a;
    /* end of critical section */
    
    //pthread_mutex_unlock(&mutex);
  }
}
