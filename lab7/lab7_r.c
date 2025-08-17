#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>      
#include <sys/stat.h>    

#define MSG_SIZE 256
#define KEY_FILE "lab7_msg_queue"

typedef struct {
    long mtype;
    char buff[MSG_SIZE];
} TMessage;

volatile int thread_exit_flag = 0;
int msgid;

void create_key_file() {
    int fd = open(KEY_FILE, O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    close(fd);
}

void* thread_function(void* arg) {
    TMessage message;
    
    while (!thread_exit_flag) {
        memset(message.buff, 0, sizeof(message.buff));
        message.mtype = 1;
        
        ssize_t rv = msgrcv(msgid, &message, sizeof(message.buff), message.mtype, IPC_NOWAIT);
        
        if (rv > 0) {
            printf("Received: %s\n", message.buff);
        } else if (rv == -1) {
            if (errno != ENOMSG) { 
                perror("msgrcv failed");
            }
        }
        
        usleep(100000);
    }
    
    return NULL;
}

int main() {
    pthread_t thread;
    key_t key;

    create_key_file();
    key = ftok(KEY_FILE, 'A');
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget failed");
        exit(1);
    }
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("pthread_create failed");
        exit(1);
    }

    printf("Receiver started. Press Enter to stop...\n");
    getchar(); 

    thread_exit_flag = 1;
    pthread_join(thread, NULL);
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID failed");
    }

    return 0;
}
