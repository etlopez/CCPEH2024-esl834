#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 60393

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    // Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("10.0.2.5"); // Server IP
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    // Communicate with server
    while(1) {
        printf("Enter command : ");
        scanf("%s" , message);

        // Send some data
        if(send(sock , message , strlen(message) , 0) < 0) {
            puts("Send failed");
            return 1;
        }

        // Receive a reply from the server
        if(recv(sock , server_reply , 2000 , 0) < 0) {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        puts(server_reply);
        memset(server_reply, 0, sizeof(server_reply)); // Clear the buffer
    }

    close(sock);
    return 0;
}
