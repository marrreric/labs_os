#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>

#define SOCKET_PATH "socket.soc"
#define BUFFER_SIZE 256

void* accept_thread(void* arg);
void* receive_thread(void* arg);
void* send_thread(void* arg);

struct request {
    int number;
    STAILQ_ENTRY(request) entries;
};

STAILQ_HEAD(stailhead, request);

int server_sock, client_sock = -1;
struct stailhead head;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int accept_running = 1;
volatile int receive_running = 1;
volatile int send_running = 1;

void sigint_handler(int sig) {
    printf("\nСервер завершает работу...\n");
    exit(0);
}

void* accept_thread(void* arg) {
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (accept_running) {
        int new_client = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (new_client == -1) {
            perror("accept");
            sleep(1);
            continue;
        }
        
        if (client_sock != -1) {
            shutdown(client_sock, SHUT_RDWR);
            close(client_sock);
        }
        client_sock = new_client;
        
        printf("Клиент подключен: %s\n", client_addr.sun_path);
        
        receive_running = 1;
        send_running = 1;
        pthread_t recv_tid, send_tid;
        pthread_create(&recv_tid, NULL, receive_thread, NULL);
        pthread_create(&send_tid, NULL, send_thread, NULL);
    }
    return NULL;
}

void* receive_thread(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (receive_running) {
        int reccount = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (reccount == -1) {
            perror("recv");
            sleep(1);
        } else if (reccount == 0) {
            printf("Клиент отключился\n");
            receive_running = 0;
            send_running = 0;
            break;
        } else {
            int num = atoi(buffer);
            struct request* req = malloc(sizeof(struct request));
            req->number = num;
            
            pthread_mutex_lock(&queue_mutex);
            STAILQ_INSERT_TAIL(&head, req, entries);
            pthread_mutex_unlock(&queue_mutex);
            
            printf("Получен запрос: %d\n", num);
        }
    }
    return NULL;
}

void* send_thread(void* arg) {
    while (send_running) {
        pthread_mutex_lock(&queue_mutex);
        if (!STAILQ_EMPTY(&head)) {
            struct request* req = STAILQ_FIRST(&head);
            STAILQ_REMOVE_HEAD(&head, entries);
            pthread_mutex_unlock(&queue_mutex);
            
            int prio = getpriority(PRIO_PROCESS, 0);
            char response[BUFFER_SIZE];
            snprintf(response, BUFFER_SIZE, "Приоритет: %d (Запрос %d)", prio, req->number);
            free(req);
            
            int sent = send(client_sock, response, strlen(response)+1, 0);
            if (sent == -1) {
                perror("send");
            } else {
                printf("Отправлен ответ: %s\n", response);
            }
        } else {
            pthread_mutex_unlock(&queue_mutex);
            sleep(1);
        }
    }
    return NULL;
}

void cleanup() {
    if (client_sock != -1) {
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }
    close(server_sock);
    unlink(SOCKET_PATH);
    
    pthread_mutex_lock(&queue_mutex);
    while (!STAILQ_EMPTY(&head)) {
        struct request* req = STAILQ_FIRST(&head);
        STAILQ_REMOVE_HEAD(&head, entries);
        free(req);
    }
    pthread_mutex_unlock(&queue_mutex);
    pthread_mutex_destroy(&queue_mutex);
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);
    
    struct sockaddr_un addr;
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(server_sock, F_SETFL, O_NONBLOCK);
    
    int optval = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);
    
    unlink(SOCKET_PATH);
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    listen(server_sock, 5);
    
    STAILQ_INIT(&head);
    
    pthread_t accept_tid;
    pthread_create(&accept_tid, NULL, accept_thread, NULL);
    
    printf("Сервер запущен. Нажмите Enter для выхода...\n");
    getchar();
    
    accept_running = 0;
    receive_running = 0;
    send_running = 0;
    
    pthread_join(accept_tid, NULL);
    cleanup();
    
    return 0;
}
