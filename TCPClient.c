#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_ADDR_LEN 128
#define FILE_NAME_LEN 128
#define FILE_SIZE_LEN 13

int connectToServer(char *addr, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(addr);

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Connection socket creating failed");
        return -1;
    }
    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error: Server connecting failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int receiveFromServer(int sockfd, char *data, int data_size) {
    int total_recv_size = 0;
    int recv_size;

    do {
        recv_size = recv(sockfd, &data[total_recv_size], data_size - total_recv_size, 0);
        total_recv_size += recv_size;
        if(recv_size < 0) {
            perror("Error: Data receiving failed");
            close(sockfd);
            return -1;
        }
    } while(total_recv_size < data_size);

    return 0;
}

int main(int argc, char *argv[]) {
    int connectionSocketDescriptor;
    char serverAddress[SERVER_ADDR_LEN];
    int port;

    FILE *filePointer;
    char fileName[FILE_NAME_LEN];
    char fileSizeBuffer[FILE_SIZE_LEN];
    int fileSize;
    char *fileData;

    if(argc != 3) {
        printf("Usage: %s [server address] [port]\n", argv[0]);
        exit(1);
    }

    strcpy(serverAddress, argv[1]);
    port = atoi(argv[2]);

    printf("Connecting to %s:%d...\n", serverAddress, port);

    if((connectionSocketDescriptor = connectToServer(serverAddress, port)) == -1) {
        perror("Error: Connecting to server failed");
        exit(1);
    }

    if(receiveFromServer(connectionSocketDescriptor, fileName, FILE_NAME_LEN) == -1) {
        perror("Error: File name receiving failed");
        exit(1);
    }

    printf("Receiving file \"%s\"...\n", fileName);

    if((filePointer = fopen(fileName, "w")) == NULL) {
        perror("Error: File accessing failed");
        exit(1);
    }

    if(receiveFromServer(connectionSocketDescriptor, fileSizeBuffer, FILE_SIZE_LEN) == -1) {
        perror("Error: File size receiving failed");
        exit(1);
    }

    fileSize = atoi(fileSizeBuffer);
    fileData = malloc(fileSize);

    if(receiveFromServer(connectionSocketDescriptor, fileData, fileSize) == -1) {
        perror("Error: File data receiving failed");
        exit(1);
    }

    if((fileSize = fwrite(fileData, 1, fileSize, filePointer)) < 0) {
        perror("Error: File writing failed");
        exit(1);
    }

    fclose(filePointer);
    close(connectionSocketDescriptor);

    printf("Receiving complete.\n");

    return 0;
}