#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 2024
#define BUFFER_SIZE 4096

void execute_commands_from_file(int sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    char buffer[BUFFER_SIZE];

    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Skip the first line (SERVER_IP is already set)
    fgets(buffer, sizeof(buffer), file);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline at the end

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            puts("Send failed");
            break;
        }

        // Wait for and print the response
        int total_received = 0, n;
        char *ptr = buffer;
        memset(buffer, 0, sizeof(buffer));
        while ((n = recv(sock, ptr, sizeof(buffer) - total_received - 1, 0)) > 0) {
            total_received += n;
            ptr += n;
            if (strstr(buffer, "\nEND\n") != NULL) {
                break;
            }
            if (total_received >= sizeof(buffer) - 1) {
                break;
            }
        }
        buffer[strcspn(buffer, "\nEND\n")] = '\0'; // Truncate END delimiter
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

    int mode = 0; // 0: Default, 1: -p, 2: -ip
    char configFile[255] = {0};

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && (i + 1) < argc) {
            mode = 1;
            strncpy(configFile, argv[i + 1], sizeof(configFile) - 1);
            i++; // Skip next argument
        } else if (strcmp(argv[i], "-ip") == 0 && (i + 1) < argc) {
            mode = 2;
            strncpy(SERVER_IP, argv[i + 1], sizeof(SERVER_IP) - 1);
            i++; // Skip next argument
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
        // Read the first line for SERVER_IP
        FILE *file = fopen(configFile, "r");
        if (!file) {
            perror("Failed to open config file");
            return 1;
        }
        if (!fgets(SERVER_IP, sizeof(SERVER_IP), file)) {
            perror("Failed to read SERVER_IP from config file");
            return 1;
        }
        SERVER_IP[strcspn(SERVER_IP, "\n")] = 0; // Remove newline at the end
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

    if (mode == 1) {
        execute_commands_from_file(sock, configFile);
    } else {
        // Normal operation or -ip mode
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

