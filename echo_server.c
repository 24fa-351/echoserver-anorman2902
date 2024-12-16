#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define ECHO_EXTRA 50
#define EXPECTED_ARGC 3
#define BACKLOG 5

int server_sock;

void error_handling(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void cleanup(int sig) {
    printf("\nShutting down server...\n\n");
    close(server_sock);
    exit(0);
}

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    struct sockaddr_in client_info;
    socklen_t client_info_len = sizeof(client_info);
    getpeername(client_sock, (struct sockaddr*)&client_info, &client_info_len);
    char *client_ip = inet_ntoa(client_info.sin_addr);
    int client_port = ntohs(client_info.sin_port);

    printf("Connected to client: %s:%d\n\n", client_ip, client_port);

    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t str_len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (str_len == 0) {
            printf("Client %s:%d disconnected.\n\n", client_ip, client_port);
            break;
        }
        if (str_len == -1) {
            perror("recv() error");
            break;
        }

        buffer[str_len] = '\0';
        printf("Received from %s:%d: %s\n", client_ip, client_port, buffer);

        char *save_ptr;
        char *line = strtok_r(buffer, "\n", &save_ptr);
        while (line != NULL) {
            char echo_msg[BUFFER_SIZE + ECHO_EXTRA];
            snprintf(echo_msg, sizeof(echo_msg), "[%s:%d]: %s\n", client_ip, client_port, line);
            if (send(client_sock, echo_msg, strlen(echo_msg), 0) == -1) {
                perror("send() error");
                break;
            }
            line = strtok_r(NULL, "\n", &save_ptr);
        }
    }

    close(client_sock);
    printf("Connection to %s:%d closed.\n\n", client_ip, client_port);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != EXPECTED_ARGC) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(1);
    }

    int port;
    if (strcmp(argv[1], "-p") == 0) {
        if (sscanf(argv[2], "%d", &port) != 1 || port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number! Please specify a valid port between 1 and 65535.\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Invalid argument! Use -p to specify port.\n");
        exit(1);
    }

    signal(SIGINT, cleanup);

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        error_handling("socket() error!");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("bind() error!");
    }

    if (listen(server_sock, BACKLOG) == -1) {
        error_handling("listen() error!");
    }

    printf("Multi-threaded echo server started on port %d. Waiting for connections...\n\n", port);

    while (1) {
        client_addr_size = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("malloc() error");
            continue;
        }

        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if (*client_sock == -1) {
            free(client_sock);
            perror("accept() error\n");
            printf("\n");
            continue;
        }

        pthread_t t_id;
        if (pthread_create(&t_id, NULL, handle_client, (void *)client_sock) != 0) {
            perror("pthread_create() error");
            close(*client_sock);
            free(client_sock);
            printf("\n");
            continue;
        }
        pthread_detach(t_id);
    }

    close(server_sock);
    return 0;
}
