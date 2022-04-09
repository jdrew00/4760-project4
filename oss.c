/*
Jacob Drew
4760 Project 3
oss.c


External resources:
https://stackoverflow.com/questions/1671336/how-does-one-keep-an-int-and-an-array-in-shared-memory-in-c
https://www.geeksforgeeks.org/ipc-using-message-queues/


*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

//struct to hold the clock
struct simClock
{
    int clockSeconds;
    int clockNS;
};

// structure for message queue
struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;



// const for random time intervals between processes
// const maxTimeBetweenNewProcsNS = 25;
// const maxTimeBetweenNewProcsSecs = 50;

// global variables
pid_t *children;
int slave_max;

// globals relating to shared memory

int shmidTime;
struct simClock *shmTime;

int maxChildren;
pid_t *children;

int main(int argc, char *argv[])
{

    int i;
    int n;
    maxChildren = 1;
    n = 1;

    // FILE *fp;
    // char *logFileName = "logfile";

    key_t shmkeyTime;

    // declare variables
    int opt;

    // getopt
    // process command line args
    while ((opt = getopt(argc, argv, ":t:h")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("Help:\n");
            printf("How to run:\n");
            printf("oss [-t n] [-h]\n");
            printf("n number of slave processes to execute\n");
            printf("If n is over 20 it will be set to 18 for safety!\n");
            // if -h is the only arg exit program
            if (argc == 2)
            {
                exit(0);
            }
            break;
        case 't':
            n = atoi(optarg);
            if (n > 18)
            {
                n = 18;
                printf("Number of processes set to 18 for safety\n");
                // printf("N: %d\n", n);
            }
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    key_t msgkey;
    int msgid;
  
    // ftok to generate unique key
    msgkey = ftok("oss.c", 65);
  
    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(msgkey, 0666 | IPC_CREAT);
    message.mesg_type = 1;
    //struct mesg_buffer message;
    char* msgbuff = "Writting msg";
    
    //snprintf(message.mesg_text, sizeof msgbuff, msgbuff);
    snprintf(message.mesg_text, 100, msgbuff);
    // msgsnd to send message
    msgsnd(msgid, &message, sizeof(message), 0);
    printf("Data send is : %s \n", message.mesg_text);


    // shared memory initialization
    shmkeyTime = ftok("./master", 118371); // arbitrary key

    shmidTime = shmget(shmkeyTime, sizeof(struct simClock), 0600 | IPC_CREAT); // create shared memory segment
    shmTime = shmat(shmidTime, NULL, 0);                               // attatch shared memory
    shmTime->clockSeconds = 0;  //set clock variables to zero
    shmTime->clockNS = 0;

    // initializing pids
    if ((children = (pid_t *)(malloc(n * sizeof(pid_t)))) == NULL)
    {
        errno = ENOMEM;
        perror("children malloc");
        exit(1);
    }
    pid_t pid;

    for (i = 0; i < n; i++)
    {
        // fork and exec one process
        pid = fork();
        if (pid == -1)
        {
            // pid == -1 means error occured
            printf("can't fork, error occured\n");
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            char *childNum = malloc(6);
            sprintf(childNum, "%d", i);

            children[i] = pid;
            char *args[] = {"./slave", (char *)childNum, "8", (char *)0};
            execvp("./slave", args);
            perror("execvp");
            exit(0);
        }
    }

    // waiting for all child processes to finish
    for (i = 0; i < n; i++)
    {
        int status;
        waitpid(children[i], &status, 0);
    }

    free(children);

    shmdt(shmTime); //detatch clock
    shmctl(shmidTime, IPC_RMID, NULL); //delete shared memory



    return 0;
}