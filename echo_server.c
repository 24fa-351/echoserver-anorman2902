#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

void error_handling(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void *handle_client(void *arg) {
    int client_sock = *(int*)arg;
    free(arg);
    char buffer[1024];

    while(1) {
        ssize_t str_len = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (str_len == 0) {
            printf("Client Disconnected.\n");
            break;
        }
        if (str_len == -1) {
            perror("recv() error");
            break;
        }

        buffer[str_len] = '\0';
        printf("Received: %s\n", buffer);

        if (strncmp(buffer, "GET /favicon.ico", 16) == 0) {
            const char *http_response = "HTTP/1.1 404 Not Found\r\n"
                                        "Content-Length: 0\r\n"
                                        "Connection: close\r\n"
                                        "\r\n";
            send(client_sock, http_response, strlen(http_response), 0);
            continue;
        }

        if (strncmp(buffer, "GET", 3) == 0) {
            const char *http_response = "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: 19\r\n"
                                        "Connection: keep-alive\r\n"
                                        "\r\n"
                                        "Hello, HTTP client!";
            if (send(client_sock, http_response, strlen(http_response), 0) == -1) {
                perror("send() error");
                break;
            }
        } else {
            if (send(client_sock, buffer, str_len, 0) == -1) {
                perror("send() error");
                break;
            }
        }
    }

    close(client_sock);
    printf("Connection closed.\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
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

    int server_sock;
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

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        error_handling("bind() error!");
    }

    if (listen(server_sock, 5) == -1) {
        error_handling("listen() error!");
    }

    printf("Multi-threaded echo server started on port %d. Waiting for connections...\n", port);

    while(1) {
        client_addr_size = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (*client_sock == -1) {
            free(client_sock);
            perror("accept() error");
            continue;
        }
        printf("Connected to a client!\n");

        pthread_t t_id;
        if (pthread_create(&t_id, NULL, handle_client, (void*)client_sock) != 0) {
            perror("pthread_create() error");
            close(*client_sock);
            free(client_sock);
            continue;
        }
        pthread_detach(t_id);
    }

    close(server_sock);
    return 0;
}
