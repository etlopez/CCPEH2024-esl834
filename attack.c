#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2024
#define BUFFER_SIZE 4096
#define HASHED_KEY "33485e06d7cc0699c8f739a7c62e2fb1c3c3caee"

void execute_commands_from_file(int sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    const char end_delim[] = "\nEND\n";
    if (!file) {
        perror("Failed to open file");
        return;
    }

    while (fgets(command, sizeof(command), file) != NULL) {
        command[strcspn(command, "\n")] = 0;
        if (send(sock, command, strlen(command), 0) < 0) {
            puts("Send failed");
            break;
        }

        // wait for and process the server's response loop
        memset(buffer, 0, sizeof(buffer));
        char *ptr = buffer;
        int total_received = 0, n;
        while ((n = recv(sock, ptr, sizeof(buffer) - total_received - 1, 0)) > 0) {
            total_received += n;
            ptr += n;
            if (strstr(buffer, end_delim) != NULL) {
                break; 
            }
            if (total_received >= sizeof(buffer) - 1) {
                break; 
            }
        }
        if (n < 0) {
            puts("Receive failed");
            break;
        }
        *strstr(buffer, end_delim) = '\0'; 
        printf("%s\n", buffer); 
    }
    fclose(file);
}


int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    const char end_delim[] = "\nEND\n";
    char SERVER_IP[16] = {0};
    int mode = 0; 
    char configFile[255] = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && (i + 1) < argc) {
            mode = 1;
            strncpy(configFile, argv[i + 1], sizeof(configFile) - 1);
            i++; 
        } else if (strcmp(argv[i], "-ip") == 0 && (i + 1) < argc) {
            mode = 2;
            strncpy(SERVER_IP, argv[i + 1], sizeof(SERVER_IP) - 1);
            i++; 
        }
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (mode == 1) {
        // read the first line for SERVER_IP
        FILE *file = fopen(configFile, "r");
        if (!file) {
            perror("Failed to open config file");
            return 1;
        }
        if (!fgets(SERVER_IP, sizeof(SERVER_IP), file)) {
            perror("Failed to read SERVER_IP from config file");
            return 1;
        }
        SERVER_IP[strcspn(SERVER_IP, "\n")] = 0; 
        fclose(file);
    }

    if (mode != 2 && strlen(SERVER_IP) == 0) {
        fprintf(stderr, "SERVER_IP not specified\n");
        return 1;
    }

    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    printf("Connected to %s\n", SERVER_IP);

    if (send(sock, HASHED_KEY, strlen(HASHED_KEY), 0) < 0) {
        puts("Failed to send auth key");
        close(sock);
        return 1;
    }

    char auth_confirmation[10];
    memset(auth_confirmation, 0, sizeof(auth_confirmation)); // Clear the buffer
    if (recv(sock, auth_confirmation, sizeof(auth_confirmation) - 1, 0) <= 0) {
        puts("Failed to receive auth confirmation");
        close(sock);
        return 1;
    }
    if (strncmp(auth_confirmation, "AUTH_OK\n", 8) != 0) {
        puts("Auth failed.");
        close(sock);
        return 1;
    }

    if (mode == 1) {
        execute_commands_from_file(sock, configFile);
    } else {
        // Normal operation or -ip mode
        while (1) {
            printf("Enter command: ");
            fflush(stdout);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "exit") == 0) {
                break;
            }
            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                puts("Send failed");
                break;
            }
            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                puts("Send failed");
                break;
            }

            // wait for the server's response
            memset(buffer, 0, sizeof(buffer)); 
            char *ptr = buffer;
            int total_received = 0, n;
            while (1) {
                n = recv(sock, ptr, sizeof(buffer) - total_received - 1, 0);
                if (n <= 0) { 
                    puts("Receive failed or connection closed");
                    break;
                }
                total_received += n;
                ptr += n;
                if (strstr(buffer, "\nEND\n") != NULL) {
                    break; 
                }
            }
            *strstr(buffer, "\nEND\n") = '\0'; 
            printf("Server reply:\n%s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        close(sock);
    }
    return 0;
}