#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>

void print_directory();
void change_dir();
void tokenize();
void signal_handler();

// TODO
// have if statements to check to see if you have more tokens than your supposed to for all commands
// Allow child proccesses to have args passed to it
// Extra credit?
// Finishing touches / cleanup
// default should be doing executable

typedef struct 
{
    int length;     // Size of the list
    int id_count;   // Size of the amount of ids in the list
    int last_index; // Index of last item
    pid_t* arr;
} PID_list;

void remove_list(pid_t member);
void add_list(pid_t member);
void free_list();
PID_list* create_list(int size);
void print_list();

char usr_name[30];
PID_list* pids;

int main(int argc, char **argv)
{

    // For handling user input name
    if(argc < 3)
    {
        strcpy(usr_name, "308sh>");
    }
    else if(argc > 3)
    {
        printf("Too many arguments\n");
        return 0;
    }
    else
    {
        strcpy(usr_name, argv[2]);
        strcat(usr_name, ">");
    }

    char buffer[75];
    char tokens[20][20];
    int tokens_count = 0;

    pids = create_list(5);

    // Function call to handle zombie processes
    signal(SIGCHLD, signal_handler);

    // While true, keep handeling user input
    while(1)
    {
        printf("%s", usr_name);
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        tokenize(buffer, tokens, &tokens_count);

        if(strcmp(tokens[0], "exit") == 0)
        {
            free_list();
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(tokens[0], "pwd") == 0)
        {
            char path[75];
            getcwd(path, sizeof(path));
            printf("%s\n", path);
        }
        else if(strcmp(tokens[0], "ls") == 0)
        {
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    printf("%s ", dir->d_name);
                }
                printf("\n");
                closedir(d);
            }
        }
        else if(strcmp(tokens[0], "pid") == 0)
        {
            int pid = getpid();
            printf("%d\n", pid);
        }
        else if(strcmp(tokens[0], "jobs") == 0)
        {
            print_list();
        }
        else if(strcmp(tokens[0], "ppid") == 0)
        {
            int pid = getppid();
            printf("%d\n", pid);
        }
        else if(strcmp(tokens[0], "cd") == 0)
        {
            if(tokens_count == 2)
            {
                change_dir(tokens[1]);
            }
            else if(tokens_count == 1)
            {
                char* home_dir;
                home_dir = getenv("HOME");
                change_dir(home_dir);
            }
            else
            {
                printf("Too many args for cd\n");
            }
        }
        else if((strcmp(tokens[0], "") == 0))
        {
            // do nothing, for if you press enter
        }
        else
        {
            // Executable
            char* token_start = tokens[0];

            fflush(stdout);
            pid_t p = fork();

            if (p < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }

            if(p == 0)
            {

                pid_t parent_id = getpid();
                printf("[%d] %s\n", parent_id, token_start);
                fflush(stdout);

                //execute
                char *args[20];
                int max_index = tokens_count;

                for (int i = 0; i < max_index; i++) {
                    args[i] = tokens[i]; // Prepare args for execvp
                }
                if(strcmp(args[tokens_count - 1], "&") == 0) args[tokens_count - 1] = NULL; // Background
                else args[tokens_count] = NULL;
                
                int status_code = execvp(token_start, args);
                
                if(status_code == -1)
                {
                    printf("Cannot execute %s: No such file or directory\n", token_start);
                }
                exit(status_code);
                //printf("%s", usr_name);

            }
            else
            {
                // Add child process id to list
                add_list(p);

                if (tokens_count > 1 && strcmp(tokens[tokens_count - 1], "&") == 0) {
                    usleep(500); // Allowing the child to print out the process before printing out the user input prompt again
                } else {
                    int exit_status;
                    waitpid(p, &exit_status, 0); // Wait for the child process
                    int actual_exit_code = WEXITSTATUS(exit_status);
                    printf("[%d] %s Exit %d\n", p, tokens[0], actual_exit_code);
                    remove_list(p);
                }
            }
        }

    }

    return 0;

}

void print_list()
{
    for(int i = 0; i < pids->length; i++)
    {
        if(pids->arr[i] != -1 && pids->arr[i] != 0)
        {
            printf("pid: %d\n", pids->arr[i]);
        }
    }
}

void remove_list(pid_t member)
{
    for(int i = 0; i <= pids->last_index; i++)  // Last index here is inclusive, meaning that you want another iteration if you are equal to the last_index number
    {
        if(pids->arr[i] == member)
        {
            pids->arr[i] = -1;
            pids->id_count--;
        }
    }
}

/* Problem with this method is that I am never getting rid of the gravestone proccess that exist in the list
*  Assuming for our use case there shouldn't be enough processes for this to matter
*/
void add_list(pid_t member)
{
    // Not enough space in array and you have to increase the size of the array
    if(pids->last_index >= pids->length - 1)
    {
        int new_size = pids->length * 2;
        pid_t* new_array = (pid_t*) realloc(pids->arr, new_size * sizeof(pid_t));
        if (new_array == NULL) {
            perror("Failed to reallocate memory for PID array");
            exit(EXIT_FAILURE);
        }
        pids->arr = new_array;
        pids->length = new_size;
    }
    pids->last_index++;
    pids->arr[pids->last_index] = member;
    pids->id_count++;
}

PID_list* create_list(int size)
{
    pid_t* new_array = (pid_t*) malloc(size * sizeof(pid_t));
    PID_list* new_list = (PID_list*) malloc(sizeof(PID_list));
    if (new_list == NULL) {
        perror("Failed to allocate memory for PID_list");
        exit(EXIT_FAILURE);
    }

    new_list->arr = new_array;
    new_list->arr = (pid_t*) malloc(size * sizeof(pid_t)); // Allocate memory for the array
    if (new_list->arr == NULL) {
        perror("Failed to allocate memory for PID array");
        free(new_list);
        exit(EXIT_FAILURE);
    }

    new_list->length = size;
    new_list->id_count = 0;
    new_list->last_index = -1;  // -1 since nothing in it yet

    return new_list;
}

void free_list()
{
    free(pids->arr);
    free(pids);
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

void signal_handler(int sig) {
    int status;
    pid_t pid;

    // Reap all terminated child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            printf("\n[%d] Exit %d\n%s", pid, exit_status, usr_name);
            fflush(stdout);
        }
        else if (WIFSIGNALED(status)) 
        {
            int term_signal = WTERMSIG(status);
            printf("[%d] Killed (%d)\n%s", pid, term_signal, usr_name);
            fflush(stdout);
        }
        remove_list(pid);
    }
}

void change_dir(char path[])
{
    // Need to check if given_str is relative or absolute
    
    if(chdir(path) == -1)
    {   
        // Unsuccessful
        printf("No such file or directory: %s\n", path);
    }

}

