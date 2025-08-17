#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("Родительский процесс: PID = %d, PPID = %d\n", getpid(), getppid());

    // Вывод аргументов командной строки
    printf("Аргументы командной строки родительского процесса:\n");
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс
        // Используем execl (передаем только первые 3 аргумента)
        execl("./lab4_d", "lab4_d", argv[1], argv[2], NULL);
        
        // Если execl вернул управление, значит произошла ошибка
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс
        printf("Родительский процесс: создан дочерний процесс с PID = %d\n", pid);

        int status;
        while (waitpid(pid, &status, WNOHANG) == 0) {
            printf("Родительский процесс: ждем завершения дочернего процесса...\n");
            usleep(500000); // Ждем 0.5 секунды (500000 микросекунд)
        }

        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else {
            printf("Дочерний процесс завершился некорректно\n");
        }
    }

    return 0;
}
