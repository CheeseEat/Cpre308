#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> 
#include <unistd.h> 
#include <pthread.h>
#include <sys/time.h>

#include "Bank.h"

struct trans { // structure for a transaction pair
    int acc_id; // account ID
    int amount; // amount to be added, could be positive or negative
};

// Linked list node
struct request
{
    struct request * next;              // pointer to the next request in the list
    int request_id;                     // request ID assigned by the main thread
    int check_acc_id;                   // account ID for a CHECK request
    struct trans * transactions;        // array of transaction data
    int num_trans;                      // number of accounts in this transaction
    struct timeval starttime, endtime;  // starttime and endtime for TIME 
};

// Linked list of jobs
struct queue
{
    struct request *head, *tail;
    int num_jobs;
};

void initialize_mutexlocks();
void tokenize();
void* thread_worker();
int check_transactions_valid(struct request* transaction);
void join_all_threads(pthread_t* worker_threads_array, int num_threads);
void initialize_workers(pthread_t* worker_threads_array, int num_threads);
void print_q();

// Tells worker threads to stop
volatile int stop_flag = 0;
pthread_mutex_t q_lock;
pthread_cond_t q_cond = PTHREAD_COND_INITIALIZER;
struct queue q;
pthread_mutex_t* mutex_locks_array;

int main(int argc, char* argv[])
{

    if(argc != 4)
    {
        printf("Arguments = %d, should be 4\n", argc);
        return 0;
    }

    // Initialization
    int num_threads = atoi(argv[1]);
    int num_accounts = atoi(argv[2]);
    char* file_path = argv[3];
    FILE* output_file;
    if(num_threads == 0 || num_accounts == 0)
    {
        printf("# of worker threads or # of accounts invalid input");
        return 0;
    }

    output_file = fopen(file_path, "w");
    if(output_file == NULL)
    {
        printf("No such file: %s", file_path);
        return 0;
    }

    initialize_accounts(num_accounts);

    char buffer[75];
    char tokens[20][20];
    int tokens_count = 0;

    // Make an array with a mutex lock for each account
    mutex_locks_array = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_accounts);
    initialize_mutexlocks(num_accounts);

    pthread_t* worker_threads_array = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    if (!worker_threads_array) {
        printf("Failed to allocate memory for worker threads array\n");
        return 1; // Exit or handle error
    }
    initialize_workers(worker_threads_array, num_threads);

    if(pthread_mutex_init(&q_lock, NULL) != 0) printf("FAILED TO initialize q mutex lock\n");

    // Initalize the queue with all null values
    q.head = NULL, q.tail = NULL, q.num_jobs = 0;

    // To keep track of the current id requested
    int request_id = 1;

    // Main Logic
    while(1)
    {
        print_q();
        fgets(buffer, sizeof(buffer), stdin);
        tokenize(buffer, tokens, &tokens_count);
        if(strcmp(tokens[0], "CHECK") == 0)
        {
            // Balance check
            struct timeval time;
            gettimeofday(&time, NULL);

            int given_id = atoi(tokens[1]);

            // // Make a task and add it to the end of the queue for a worker thread to pick up
            struct request *cur_request = (struct request *)malloc(sizeof(struct request));
            *cur_request = (struct request){.request_id = request_id, .check_acc_id = given_id, .starttime = time};
            pthread_mutex_lock(&q_lock);
            struct request* last_request = q.tail;
            if(last_request == NULL)
            {
                q.head = cur_request;
                q.tail = cur_request;
            }
            else
            {
                last_request->next = cur_request;
                q.tail = cur_request;
            }
            //printf("head: %d tail %d\n", q.head->request_id, q.tail->request_id);
            printf("ID %d\n", request_id);
            request_id++;
            pthread_cond_broadcast(&q_cond);
            pthread_mutex_unlock(&q_lock);
        }
        else if(strcmp(tokens[0], "TRANS") == 0)
        {
            // Request transaction
            struct timeval time;
            gettimeofday(&time, NULL);

            // Go through and get all the transactions from the given string
            int number_transactions = (tokens_count - 1) / 2;
            struct trans transactions[number_transactions]; // Minus 1 for the first prompt in the string
            for(int i = 1; i < tokens_count; i+=2)
            {
                int account_id = atoi(tokens[i]);
                int ammount = atoi(tokens[i+1]);
                struct trans temp_transaction = {.acc_id = account_id, .amount = ammount};
                transactions[i/2] = temp_transaction;
            }

            // Add it to the queue
            struct request *cur_request = (struct request *)malloc(sizeof(struct request));
            *cur_request = (struct request){.request_id = request_id, .transactions = transactions, .num_trans = number_transactions, .starttime = time};
            request_id++;
            pthread_cond_broadcast(&q_cond);
            pthread_mutex_lock(&q_lock);
            struct request* last_request = q.tail;
            last_request->next = cur_request;
            q.tail = cur_request;
            pthread_mutex_unlock(&q_lock);
        }
        else if (strcmp(tokens[0], "END") == 0)
        {
            stop_flag = 1;
            pthread_cond_broadcast(&q_cond);  // Wake all threads to terminate
            join_all_threads(worker_threads_array, num_threads);
            return 0;
        }
        else
        {
            printf("INVALID INPUT\n");
        }
    }

    // Cleanup
    fclose(output_file);
    free(mutex_locks_array);
    free(worker_threads_array);
    free_accounts();

}

void* thread_worker(void* arg)
{

    // Loop untill we recieve the stop flag
    while(!stop_flag)
    {
        
        pthread_mutex_lock(&q_lock);

        while(q.head == NULL && !stop_flag) pthread_cond_wait(&q_cond, &q_lock);

        if (stop_flag && q.head == NULL) {
            pthread_mutex_unlock(&q_lock);
            break;
        }

        // Grab the job at the front of the queue and see if anyone has it locked.
        struct request* cur_trans = q.head;
        struct request* prev_trans = NULL;
        pthread_mutex_t cur_lock = mutex_locks_array[cur_trans->request_id];
        
        // Go untill you find a request that needs work
        while(pthread_mutex_trylock(&cur_lock) != 0)
        {
            prev_trans = cur_trans;
            cur_trans = cur_trans->next;
            cur_lock = mutex_locks_array[cur_trans->request_id];
        }

        printf("HERHE");

        // Unlock and adjust the q which needs a lock of its own
        struct timeval endtime;
        gettimeofday(&endtime, NULL);
        if(cur_trans->check_acc_id != 0)
        {
            int amt = read_account(cur_trans->check_acc_id);
            printf("%d BAL %d TIME %ld.%06ld %ld.%06ld\n", cur_trans->request_id, amt, cur_trans->starttime.tv_sec, cur_trans->starttime.tv_usec, endtime.tv_sec, endtime.tv_usec);
        }
        else
        {
            // You have to go through all of the accounts to see if they have sufficent values to carry out the transaction
            int acct_num;
            if((acct_num = check_transactions_valid(cur_trans)) == 0)
            {
                // Carry out the transaction/s
                for(int j = 0; j < cur_trans->num_trans; j++)
                {
                    struct trans temp_trans = cur_trans->transactions[j];
                    pthread_mutex_t temp_lock = mutex_locks_array[j];
                    pthread_mutex_lock(&temp_lock);
                    write_account(temp_trans.acc_id, temp_trans.amount);
                    pthread_mutex_unlock(&temp_lock);
                }
            }
            // Otherwise the transaction is void
            else
            {
                printf("%d ISF %d TIME %ld.%06ld %ld.%06ld\n", cur_trans->request_id, acct_num, cur_trans->starttime.tv_sec, cur_trans->starttime.tv_usec, endtime.tv_sec, endtime.tv_usec);
            }
        }

        printf("WHATTTT");

        // Adjust the q
        if(cur_trans == q.head)
        {
            q.head->next = cur_trans->next;
        }
        else
        {
            prev_trans->next = cur_trans->next;
        }

        pthread_mutex_unlock(&q_lock);
        free(cur_trans);
        pthread_mutex_unlock(&cur_lock);

    }

    pthread_exit(NULL);

}

int compare_transactions(const void* a, const void* b)
{
    return ((struct trans*)a)->acc_id - ((struct trans*)b)->acc_id;
}

int check_transactions_valid(struct request* transaction)
{
    // Need to go through all the account checking their balance to see if sufficient
    qsort(transaction->transactions, transaction->num_trans, sizeof(struct trans), compare_transactions);
    for(int i = 0; i < transaction->num_trans; i++)
    {
        struct trans temp_trans = transaction->transactions[i];
        int acc_id = temp_trans.acc_id;
        pthread_mutex_lock(&mutex_locks_array[acc_id]);
        int temp_acct_balance = read_account(temp_trans.acc_id);
        pthread_mutex_unlock(&mutex_locks_array[acc_id]);
        if(temp_acct_balance - temp_trans.amount < 0) return temp_trans.acc_id;
    }

    return 0;

}

void tokenize(char str[], char tokens[][20], int* count)
{
    
    char* token = strtok(str, " ");
    int i = 0;
    char* pos;
    while (token) {
        pos = strchr(token, '\n');
        if(pos != NULL) 
        {
            token[pos - token] = '\0';
        }
        strcpy(tokens[i], token);
        token = strtok(NULL, " ");
        i++;
    }

    *count = i;

}

void* chop(void* arg)
{
    pthread_exit(NULL);
}

void initialize_mutexlocks(int num_accounts)
{
    for(int i = 0; i < num_accounts; i++)
    {
        if(pthread_mutex_init(&mutex_locks_array[i], NULL) != 0) printf("failed to create mutex\n");
    }
}

void initialize_workers(pthread_t* worker_threads_array, int num_threads)
{
    for(int i = 0; i < num_threads; i++)
    {
        if(pthread_create(&worker_threads_array[i], NULL, thread_worker, NULL) != 0) printf("failed to create mutex\n");
    }
}

void join_all_threads(pthread_t* worker_threads_array, int num_threads)
{
    for(int i = 0; i < num_threads; i++)
    {
        pthread_join(worker_threads_array[i], NULL);
    }
}

void print_q()
{
    struct request* current = q.head;
    while(current != NULL)
    {
        printf("%d\n", current->request_id);
        current = current->next;
    }
}