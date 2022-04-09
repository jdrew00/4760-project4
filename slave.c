/*
Jacob Drew
4760 Project 3
userProcess.c

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/msg.h>

// structure for message queue
struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char *argv[])
{
    key_t msgkey;
    int msgid;
  
    // ftok to generate unique key
    msgkey = ftok("oss.c", 65);
  
    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(msgkey, 0666 | IPC_CREAT);
  
    // msgrcv to receive message
    msgrcv(msgid, &message, sizeof(message), 1, 0);
  
    // display the message
    printf("Data Received is : %s \n", 
                    message.mesg_text);
  
    // to destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);
    printf("hello world");
    return 0;    
}