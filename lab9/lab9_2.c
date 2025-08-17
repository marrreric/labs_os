#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
int main() {
    if (geteuid() != 0) {
        printf("Запустите программу с sudo!\n");
        return 1;
    }
    const char *file = "lab9_1";

    cap_t caps = cap_get_file(file);
    if (!caps) {
        if (errno == ENODATA) {
            printf("У файла '%s' нет установленных возможностей.\n", file);
            return 0;
        } else {
            perror("cap_get_file");
            return 1;
        }
    }

    char *str = cap_to_text(caps, NULL);
    printf("Текущие возможности: %s\n", str);
    cap_free(str);
    cap_free(caps);

    printf("Введите новые возможности (пример: cap_net_bind_service=+eip): ");
    char buf[256];
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';

    cap_t new_caps = cap_from_text(buf);
    if (!new_caps || cap_set_file(file, new_caps)) {
        perror("Ошибка установки");
        if (new_caps) cap_free(new_caps);
        return 1;
    }

    printf("Успешно обновлено!\n");
    cap_free(new_caps);
    return 0;
}
