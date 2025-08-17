#define _GNU_SOURCE  // Для pipe2() в Linux
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t thread1_finish = 0;
volatile sig_atomic_t thread2_finish = 0;
int pipefd[2];

void sigint_handler(int sig) {
    thread1_finish = 1;
    thread2_finish = 1;
}

void* thread1_func(void* arg) {
    char buffer[256];
    int mode = *((int*)arg);
    
    while (!thread1_finish) {
        int prio = getpriority(PRIO_PROCESS, 0);
        snprintf(buffer, sizeof(buffer), "Приоритет процесса: %d", prio);
        
        ssize_t rv = write(pipefd[1], buffer, strlen(buffer) + 1);
        
        if (rv == -1) {
            perror("Ошибка записи");
            if (mode != 1 && errno == EAGAIN) {
                printf("Канал заполнен. Ожидание...\n");
            }
            if (errno == EPIPE) {
                break;
            }
        } else if (rv > 0) {
            printf("Поток записи: отправлено %zd байт\n", rv);
        }
        
        sleep(1);
    }
    return NULL;
}

void* thread2_func(void* arg) {
    char buffer[256];
    int mode = *((int*)arg);
    
    while (!thread2_finish) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t rv = read(pipefd[0], buffer, sizeof(buffer));
        
        if (rv == -1) {
            if (errno == EAGAIN) {
                if (mode != 1) {
                    printf("Канал пуст. Ожидание...\n");
                }
                usleep(100000);
                continue;
            }
            perror("Ошибка чтения");
            break;
        } else if (rv == 0) {
            printf("Канал закрыт\n");
            break;
        } else if (rv > 0) {
            printf("Поток чтения: получено: %s\n", buffer);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    printf("Программа запущена\n");
    
    if (argc != 2) {
	printf("Выберите режим работы канала через");
	printf(" ./lab3 1|2|3, где \n");
        printf("1 - pipe() с блокировками\n");
        printf("2 - pipe2() без блокировок\n");
        printf("3 - pipe() + fcntl() без блокировок\n");
        return 1;
    }
    
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    int mode = atoi(argv[1]);
    if (mode < 1 || mode > 3) {
        printf("Неверный режим. Используйте 1, 2 или 3\n");
        return 1;
    }
    
    if (mode == 2) {
        // pipe2() с O_NONBLOCK
        if (pipe2(pipefd, O_NONBLOCK) == -1) {
            perror("Ошибка pipe2()");
            if (pipe(pipefd) == -1) {
                perror("Ошибка создания канала");
                return 1;
            }
            if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == -1 ||
                fcntl(pipefd[1], F_SETFL, O_NONBLOCK) == -1) {
                perror("Ошибка fcntl");
                return 1;
            }
        }
    } else {
        if (pipe(pipefd) == -1) {
            perror("Ошибка создания канала");
            return 1;
        }

        if (mode == 3) {
            if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == -1 ||
                fcntl(pipefd[1], F_SETFL, O_NONBLOCK) == -1) {
                perror("Ошибка fcntl");
                return 1;
            }
        }
    }
    
    pthread_t writer, reader;
    
    if (pthread_create(&writer, NULL, thread1_func, &mode)) {
        perror("Ошибка создания потока записи");
        return 1;
    }
    
    if (pthread_create(&reader, NULL, thread2_func, &mode)) {
        perror("Ошибка создания потока чтения");
        return 1;
    }
    
    printf("Нажмите Enter для выхода...\n");
    getchar();
    
    thread1_finish = 1;
    thread2_finish = 1;
    
    close(pipefd[0]);
    close(pipefd[1]);
    
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    
    printf("Программа завершена\n");
    return 0;
}
