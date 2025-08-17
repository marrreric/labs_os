#include "mylib.h"
#include <stdio.h>

int main() {
    int prio = my_getpriority(PRIO_PROCESS, 0);
    printf("Static: Priority = %d\n", prio);
    return 0;
}
