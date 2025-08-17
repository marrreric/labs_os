#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <pthread.h>

volatile sig_atomic_t flag = 0;
sem_t *sem_write;
sem_t *sem_read;
int shm_fd;
void *addr;
pthread_t thread_id;

void cleanup() {
    printf("\nCleaning up resources...\n");
    
    if (sem_read) {
        sem_close(sem_read);
        sem_unlink("/lab6_sem_read");
    }
    if (sem_write) {
        sem_close(sem_write);
        sem_unlink("/lab6_sem_write");
    }
    
    if (addr) munmap(addr, sizeof(int));
    if (shm_fd >= 0) {
        close(shm_fd);
        shm_unlink("/lab6_shm");
    }
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        flag = 1;
        cleanup();
        exit(0);
    }
}

void* thread_func(void* arg) {
    while (!flag) {
        int priority = getpriority(PRIO_PROCESS, 0);
        printf("Writer priority: %d\n", priority);
        
        memcpy(addr, &priority, sizeof(priority));
        
        sem_post(sem_write);
        sem_wait(sem_read); 
        
        sleep(1);
    }
    return NULL;
}

int main() {
    signal(SIGINT, sig_handler);
    atexit(cleanup);
    
    sem_unlink("/lab6_sem_write");
    sem_unlink("/lab6_sem_read");
    shm_unlink("/lab6_shm");
    
    shm_fd = shm_open("/lab6_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    
    addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    sem_write = sem_open("/lab6_sem_write", O_CREAT | O_EXCL, 0666, 0);
    sem_read = sem_open("/lab6_sem_read", O_CREAT | O_EXCL, 0666, 0);
    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_create(&thread_id, NULL, thread_func, NULL) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    
    printf("Writer started. Press Enter to stop or Ctrl+C to force exit...\n");
    getchar();
    
    flag = 1;
    pthread_join(thread_id, NULL);
    cleanup();
    
    return 0;
}
