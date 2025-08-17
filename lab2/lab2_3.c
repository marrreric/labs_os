#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

typedef struct {
    int flag;
    char sym;
} targs;

pthread_spinlock_t spinlock;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void atomic_print(const char* msg) {
    pthread_mutex_lock(&print_mutex);
    printf("%s\n", msg);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
}

void* thread_func(void* arg) {
    targs* args = (targs*)arg;
    char buffer[256];
    int error_printed = 0; // Флаг для однократного вывода ошибки
    
    snprintf(buffer, sizeof(buffer), "Thread %c started", args->sym);
    atomic_print(buffer);
    
    while (args->flag == 0) {
        // Попытка захватить спинлок
        int spin_result;
        while ((spin_result = pthread_spin_trylock(&spinlock)) != 0) {
            if (args->flag != 0) {
                snprintf(buffer, sizeof(buffer), "Thread %c finished (aborted)", args->sym);
                atomic_print(buffer);
                return NULL;
            }
            
            if (!error_printed) {
                snprintf(buffer, sizeof(buffer), 
                         "Thread %c: %s (waiting for lock)", 
                         args->sym, strerror(spin_result));
                atomic_print(buffer);
                error_printed = 1;
            }
            
            usleep(100000);
        }
        error_printed = 0; 
        
        //Начало КУ
        snprintf(buffer, sizeof(buffer), "Thread %c ENTERED critical section", args->sym);
        atomic_print(buffer);
        
        char symbols[11] = {0};
        for (int i = 0; i < 10 && args->flag == 0; i++) {
            symbols[i] = args->sym;
            sleep(1);
        }
        atomic_print(symbols);
        
        snprintf(buffer, sizeof(buffer), "Thread %c LEFT critical section", args->sym);
        atomic_print(buffer);
        // Конеу КУ
        
        pthread_spin_unlock(&spinlock);
        
        // Работа вне КУ
        if (args->flag == 0) {
            sleep(1);
        }
    }
    
    snprintf(buffer, sizeof(buffer), "Thread %c finished normally", args->sym);
    atomic_print(buffer);
    return NULL;
}

int main() {
    atomic_print("Program started");
    
    pthread_t id1, id2;
    targs arg1 = {0, '1'};
    targs arg2 = {0, '2'};
    
    if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
        perror("Spinlock init failed");
        return 1;
    }
    
    pthread_create(&id1, NULL, thread_func, &arg1);
    pthread_create(&id2, NULL, thread_func, &arg2);
    
    atomic_print("Press Enter to stop...");
    getchar();
    
    arg1.flag = arg2.flag = 1;
    
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    
    pthread_spin_destroy(&spinlock);
    pthread_mutex_destroy(&print_mutex);
    atomic_print("Program finished");
    return 0;
}
