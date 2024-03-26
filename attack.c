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

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected\n");

    while (1) {
        printf("Enter command: ");
        fflush(stdout); // Make sure "Enter command" is printed immediately

        // Get the command from the user
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline at the end

        // Exit loop if command is "exit"
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Send the command
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            puts("Send failed");
            break;
        }

        // Receive a reply from the server
        int received = 0;
        if ((received = recv(sock, buffer, sizeof(buffer) - 1, 0)) < 0) {
            puts("recv failed");
            break;
        }
        buffer[received] = '\0'; // Null-terminate the received data
        printf("Server reply :\n%s\n", buffer);

        // Clear the buffer for the next command
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}
