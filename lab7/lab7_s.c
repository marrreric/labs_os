#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h> 
#define MSG_SIZE 256
#define KEY_FILE "lab7_msg_queue"

volatile int thread_exit_flag = 0;
int msgid;

typedef struct {
    long mtype;
    char buff[MSG_SIZE];
} TMessage;

void* thread_function(void* arg) {
    TMessage message;
    
    while (!thread_exit_flag) {
        int priority = getpriority(PRIO_PROCESS, 0);
        printf("Current priority: %d\n", priority);
        
        message.mtype = 1;
        int len = snprintf(message.buff, MSG_SIZE, "Priority: %d", priority);
        
        int rv = msgsnd(msgid, &message, len + 1, IPC_NOWAIT);
        if (rv == -1) {
            perror("msgsnd failed");
        } else {
            printf("Message sent successfully\n");
        }
        
        sleep(1);
    }
    return NULL;
}
 
int main() {
    pthread_t thread;
    key_t key;
    
    int fd = open(KEY_FILE, O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    close(fd);
    
    key = ftok(KEY_FILE, 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }
    
    pthread_create(&thread, NULL, thread_function, NULL);
    
    printf("Sender started. Press Enter to stop...\n");
    getchar();
    
    thread_exit_flag = 1;
    pthread_join(thread, NULL);
    
    return 0;
}
