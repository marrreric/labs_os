#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(1000),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (errno == EACCES) {
            printf("Ошибка: Недостаточно прав для привязки к порту 1000\n");
        } else {
            perror("bind");
        }
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Успешная привязка к порту 1000\n");
    close(sockfd);
    return 0;
}
