#include <stdio.h>
#include <dlfcn.h>
#include <sys/resource.h> 

int main() {
    void *handle;
    int (*my_getpriority)(int, id_t); 
    char *error;

    handle = dlopen("./libmylib.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return 1;
    }

    my_getpriority = (int (*)(int, id_t))dlsym(handle, "my_getpriority");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "dlsym: %s\n", error);
        return 1;
    }

    int prio = my_getpriority(PRIO_PROCESS, 0); 
    printf("dlopen: Priority = %d\n", prio);

    dlclose(handle);
    return 0;
}
