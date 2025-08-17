#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

volatile sig_atomic_t flag = 0;
sem_t *sem_write;
sem_t *sem_read;
int shm_fd;
void *addr;
pthread_t thread_id;

void cleanup() {
    printf("\nCleaning up resources...\n");
    
    if (sem_read) sem_close(sem_read);
    if (sem_write) sem_close(sem_write);
    
    if (addr) munmap(addr, sizeof(int));
    if (shm_fd >= 0) close(shm_fd);
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        flag = 1;
        cleanup();
        exit(0);
    }
}

void* thread_func(void* arg) {
    int received_priority;
    while (!flag) {
        sem_wait(sem_write);
        
        memcpy(&received_priority, addr, sizeof(received_priority));
        printf("Received priority: %d\n", received_priority);
        
        sem_post(sem_read);  
    }
    return NULL;
}

int main() {
    signal(SIGINT, sig_handler);
    atexit(cleanup);
    
    shm_fd = shm_open("/lab6_shm", O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    sem_write = sem_open("/lab6_sem_write", 0);
    sem_read = sem_open("/lab6_sem_read", 0);
    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_create(&thread_id, NULL, thread_func, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    
    printf("Reader started. Press Enter to stop or Ctrl+C to force exit...\n");
    getchar();
    
    flag = 1;
    pthread_join(thread_id, NULL);
    cleanup();
    
    return 0;
}
