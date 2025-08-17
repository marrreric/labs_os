#include "mylib.h"

int my_getpriority(int which, id_t who) {
    return getpriority(which, who);
}
