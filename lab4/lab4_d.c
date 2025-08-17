#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("Дочерний процесс: PID = %d, PPID = %d\n", getpid(), getppid());

    //вывод первых 3 аргументов, так как используется execl
    printf("Аргументы, переданные в дочерний процесс:\n");
    for (int i = 0; i < argc && i < 3; i++) { 
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    // завершение с кодом 5
    for (int i = 0; i < 5; i++) {
        sleep(1);
        printf("Дочерний процесс: прошло %d секунд\n", i + 1);
    }

    exit(5);
}
