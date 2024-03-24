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

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
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
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        send(sock, buffer, strlen(buffer), 0);

        int received = 0;
        while ((received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[received] = '\0';
            printf("%s", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        printf("\n");
    }

    close(sock);
    return 0;
}
