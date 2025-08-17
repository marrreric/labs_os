#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#define SOCKET_PATH "socket.soc"
#define BUFFER_SIZE 256

void* connect_thread(void* arg);
void* send_thread(void* arg);
void* recv_thread(void* arg);

int client_sock = -1;
volatile int connect_running = 1;
volatile int send_running = 1;
volatile int recv_running = 1;
char client_socket_path[108];

void* connect_thread(void* arg) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);
    
    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    snprintf(client_socket_path, sizeof(client_socket_path), "/tmp/client_%d", getpid());
    strncpy(client_addr.sun_path, client_socket_path, sizeof(client_addr.sun_path)-1);
    
    if (bind(client_sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        perror("bind клиента");
        return NULL;
    }
    
    while (connect_running) {
        if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("connect");
            sleep(1);
        } else {
            struct sockaddr_un connected_addr;
            socklen_t len = sizeof(connected_addr);
            getsockname(client_sock, (struct sockaddr*)&connected_addr, &len);
            printf("Адрес клиента: %s\n", connected_addr.sun_path);
            
            pthread_t send_tid, recv_tid;
            pthread_create(&send_tid, NULL, send_thread, NULL);
            pthread_create(&recv_tid, NULL, recv_thread, NULL);
            
            connect_running = 0;
            break;
        }
    }
    return NULL;
}

void* send_thread(void* arg) {
    int request_num = 1;
    while (send_running) {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%d", request_num++);
        int sent = send(client_sock, buffer, strlen(buffer)+1, 0);
        if (sent == -1) {
            perror("send");
        } else {
            printf("Отправлен запрос: %s\n", buffer);
        }
        sleep(1);
    }
    return NULL;
}

void* recv_thread(void* arg) {
    char buffer[BUFFER_SIZE];
    while (recv_running) {
        int reccount = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (reccount == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                sleep(1);
                continue;
            }
            perror("recv");
            break;
        } else if (reccount == 0) {
            printf("Сервер отключился\n");
            recv_running = 0;
            send_running = 0;
            break;
        } else {
            printf("Получен ответ: %s\n", buffer);
        }
    }
    return NULL;
}

void cleanup() {
    if (client_sock != -1) {
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }
    if (strlen(client_socket_path)) {
        unlink(client_socket_path);
    }
}

int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(client_sock, F_SETFL, O_NONBLOCK);
    
    pthread_t connect_tid;
    pthread_create(&connect_tid, NULL, connect_thread, NULL);
    
    printf("Клиент запущен. Нажмите Enter для выхода...\n");
    getchar();
    
    connect_running = 0;
    send_running = 0;
    recv_running = 0;
    
    pthread_join(connect_tid, NULL);
    cleanup();
    
    return 0;
}
