#ifndef MYLIB_H
#define MYLIB_H

#include <sys/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

int my_getpriority(int which, id_t who);

#ifdef __cplusplus
}
#endif

#endif
