#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLIENT_CONN_WAIT_QUEUE_SIZE 3
#define FILE_NAME_LEN 128
#define FILE_SIZE_LEN 13

int listenAndAcceptClient(int port) {
    struct sockaddr_in local_addr;
    struct sockaddr_in client_addr;
    int client_addr_len;
    int listen_sockfd;
    int connect_sockfd;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((listen_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Listen socket creating failed");
        return -1;
    }
    if(bind(listen_sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Error: Socket binding failed");
        close(listen_sockfd);
        return -1;
    }
    if(listen(listen_sockfd, CLIENT_CONN_WAIT_QUEUE_SIZE) < 0) {
        perror("Error: Client listening failed");
        close(listen_sockfd);
        return -1;
    }
    if((connect_sockfd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
        perror("Error: Client accepting failed");
        return -1;
    }


    close(listen_sockfd);

    return connect_sockfd;
}

int sendToClient(int connection_sockfd, char *data, int data_size) {
    int total_send_size = 0;
    int send_size;

    do {
        send_size = send(connection_sockfd, &data[total_send_size], data_size - total_send_size, 0);
        total_send_size += send_size;
        if(send_size < 0) {
            perror("Error: Data sending failed");
            close(connection_sockfd);
            return -1;
        }
    } while(total_send_size < data_size);

    return 0;
}

int main(int argc, char *argv[]) {
    int connectionSocketDescriptor;
    int port;

    FILE *filePointer;
    char fileName[FILE_NAME_LEN] = {};
    int fileSize;
    char fileSizeString[FILE_SIZE_LEN] = {};
    char *fileData;

    if(argc != 3) {
        printf("Usage: %s [port] [file name]\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);
    strcpy(fileName, argv[2]);

    printf("Waiting on port %d to send file \"%s\"...\n", port, fileName);

    
    if((filePointer = fopen(fileName, "r")) == NULL) {
        perror("Error: File accessing failed");
        exit(1);
    }

    fseek(filePointer, 0, SEEK_END);
    fileSize = ftell(filePointer);
    fileData = malloc(fileSize);
    sprintf(fileSizeString, "%d", fileSize);
    fseek(filePointer, 0, SEEK_SET);

    if((fileSize = fread(fileData, 1, fileSize, filePointer)) < 0) {
        perror("Error: File reading failed");
        exit(1);
    }
    fclose(filePointer);
    
    if((connectionSocketDescriptor = listenAndAcceptClient(port)) == -1) {
        perror("Error: Listening socket ready failed");
        exit(1);
    }

    if(sendToClient(connectionSocketDescriptor, fileName, FILE_NAME_LEN) == -1) {
        perror("Error: File name sending failed");
        exit(1);
    }

    if(sendToClient(connectionSocketDescriptor, fileSizeString, FILE_SIZE_LEN) == -1) {
        perror("Error: File size sending failed");
        exit(1);
    }

    printf("Sending file...\n");

    if(sendToClient(connectionSocketDescriptor, fileData, fileSize) == -1) {
        perror("Error: File data sending failed");
        exit(1);
    }

    close(connectionSocketDescriptor);

    printf("Sending complete.\n");

    return 0;
}