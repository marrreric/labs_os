#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int flag;
    char sym;
} targs;

static void* thread_func1(void* arg) {
    targs* args = (targs*) arg;
    while (args->flag == 0) {
        putchar(args->sym);
        fflush(stdout);
        sleep(1);
    }
    
    int* ret = malloc(sizeof(int));
    *ret = 1;
    pthread_exit(ret);
}

static void* thread_func2(void* arg) {
    targs* args = (targs*) arg;
    while (args->flag == 0) {
        putchar(args->sym);
        fflush(stdout);
        sleep(1);
    }
    
    int* ret = malloc(sizeof(int));
    *ret = 2;
    pthread_exit(ret);
}
/* Contention scope (область конкуренции) определяет,
 как потоки конкурируют за процессорное время:
PTHREAD_SCOPE_SYSTEM:Потоки конкурируют(contend) за CPU со всеми потоками в системе,
 управление планированием осуществляется операционной системой,
 модель1:1 : один поток ядра на один пользовательский поток
PTHREAD_SCOPE_PROCESS:Потоки конкурируют(contend) только с другими потоками того же процесса,
 управление планированием осуществляется самой программой,
 модель M:N : много пользовательских потоков на меньшее количество потоков ядра
*/
void print_scope(pthread_attr_t *attr) {
    int scope;
    pthread_attr_getscope(attr, &scope);
    if (scope == PTHREAD_SCOPE_SYSTEM) {
        printf("Thread scope: SYSTEM (contending for CPU resources with all processes)\n");
    } else if (scope == PTHREAD_SCOPE_PROCESS) {
        printf("Thread scope: PROCESS (contending for CPU resources only within this process)\n");
    } else {
        printf("Thread scope: UNKNOWN\n");
    }
}

int main() {
    pthread_t id1, id2;
    targs arg1 = {0, '1'}, arg2 = {0, '2'};
    int *exitcode1, *exitcode2;
    pthread_attr_t attr1, attr2;
    
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);

    printf("1. Default contention scope:\n");
    print_scope(&attr1);
/* В Linux pthreads использует только PTHREAD_SCOPE_SYSTEM (1:1 модель),
 поэтому переключение на PTHREAD_SCOPE_PROCESS (M:N) невозможно.
 Это архитектурное ограничение ядра Linux.*/
    printf("\n2. Trying to change scope to PTHREAD_SCOPE_PROCESS...\n");
    if (pthread_attr_setscope(&attr1, PTHREAD_SCOPE_PROCESS) != 0) {
        printf("   Warning: System doesn't support PROCESS scope, using SYSTEM\n");
        pthread_attr_setscope(&attr1, PTHREAD_SCOPE_SYSTEM);
    }
    pthread_attr_setscope(&attr2, PTHREAD_SCOPE_SYSTEM);

    printf("\n3. Current contention scopes:\n");
    printf("Thread 1: ");
    print_scope(&attr1);
    printf("Thread 2: ");
    print_scope(&attr2);

    printf("\n4. Creating threads...\n");
    pthread_create(&id1, &attr1, thread_func1, &arg1);
    pthread_create(&id2, &attr2, thread_func2, &arg2);

    printf("Threads are running. Press Enter to stop...\n");
    getchar();

    arg1.flag = 1;
    arg2.flag = 1;

    pthread_join(id1, (void**)&exitcode1);
    pthread_join(id2, (void**)&exitcode2);

    printf("\nThreads exited with codes: %d and %d\n", *exitcode1, *exitcode2);

    free(exitcode1);
    free(exitcode2);
    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);

    return 0;
}
