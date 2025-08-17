#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int flag;
    char sym;
} targs;

void* thread_func(void* arg) {
    targs* args = (targs*)arg;
    printf("Thread %c started\n", args->sym);
    
    while (args->flag == 0) {
        // Начало критического участка
        putchar(args->sym);
        fflush(stdout);
        sleep(1);
        // Конец критического участка
        
        // Работа вне критического участка
        sleep(1);
    }
    
    printf("Thread %c finished\n", args->sym);
    return NULL;
}

int main() {
    printf("Program started\n");
    
    pthread_t id1, id2;
    targs arg1 = {0, '1'};
    targs arg2 = {0, '2'};
    
    pthread_create(&id1, NULL, thread_func, &arg1);
    pthread_create(&id2, NULL, thread_func, &arg2);
    
    printf("Program waiting for key press...\n");
    getchar();
    printf("Key pressed\n");
    
    arg1.flag = 1;
    arg2.flag = 1;
    
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    
    printf("Program finished\n");
    return 0;
}
