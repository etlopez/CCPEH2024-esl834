#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "10.0.2.5"
#define PORT 2024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[4096];
    const char end_delim[] = "\nEND\n";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected\n");

    while (1) {
        printf("Enter command: ");
        fflush(stdout);

        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline at the end

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            puts("Send failed");
            break;
        }

        memset(buffer, 0, sizeof(buffer));
        char *ptr = buffer;
        int total_received = 0, n;
        while ((n = recv(sock,        ptr, sizeof(buffer) - total_received - 1, 0)) > 0) {
            total_received += n;
            ptr += n;
            // Check if the end delimiter is in the buffer
            if (strstr(buffer, "\nEND\n") != NULL) {
                break; // Break if end delimiter is found
            }
            // Ensure we don't overflow the buffer
            if (total_received >= sizeof(buffer) - 1) {
                break;
            }
        }

        if (n < 0) {
            puts("Receive failed");
            break;
        }

        // Replace the delimiter with a null terminator to end the string
        *strstr(buffer, "\nEND\n") = '\0';
        printf("Server reply:\n%s\n", buffer);

        // Clear the buffer for the next command
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}

