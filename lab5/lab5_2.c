#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>

#define SEM_NAME "/semaphorik"
#define FILE_NAME "file_for_lab5.txt"

int main() {
    sem_t *sem;
    FILE *file;
    int ch;
    int stdin_flags;

    stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK);

    sem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        fcntl(STDIN_FILENO, F_SETFL, stdin_flags);
        exit(EXIT_FAILURE);
    }

    file = fopen(FILE_NAME, "a");
    if (!file) {
        perror("fopen failed");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        fcntl(STDIN_FILENO, F_SETFL, stdin_flags);
        exit(EXIT_FAILURE);
    }

    while (1) {
        ch = getchar();
        if (ch == '\n') {
            break;
        }

        sem_wait(sem);

        for (int i = 0; i < 10; i++) {
            fprintf(file, "2");
            fflush(file);
            printf("2");
            fflush(stdout);
            sleep(1);

            ch = getchar();
            if (ch == '\n') {
                sem_post(sem); 
                goto cleanup;
            }
        }

        sem_post(sem);

        sleep(1);
    }
cleanup:
    fclose(file);

    sem_close(sem);
    sem_unlink(SEM_NAME);

    fcntl(STDIN_FILENO, F_SETFL, stdin_flags);

    return EXIT_SUCCESS;
}
