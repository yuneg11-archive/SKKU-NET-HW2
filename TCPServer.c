#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#define CLIENT_CONN_WAIT_QUEUE_SIZE 3

int listenFromClient(int port) {
    struct sockaddr_in local_addr;
    int sockfd;

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Listen socket creating failed");
        return -1;
    }
    if(bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("Error: Socket binding failed");
        close(sockfd);
        return -1;
    }
    if(listen(sockfd, CLIENT_CONN_WAIT_QUEUE_SIZE) < 0) {
        perror("Error: Client listening failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int sendToClient(int listen_sockfd, char *data, int data_size) {
    struct sockaddr_in client_addr;
    int client_addr_len;
    int sockfd;

    if((sockfd = accept(listen_sockfd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
        perror("Error: Client accepting failed");
        return -1;
    }
    if(send(sockfd, data, data_size, 0) != data_size) {
        perror("Error: Data sending failed");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    int connectionSocketDescriptor;

    FILE *filePointer;
    int fileSize;
    char fileSizeString[13];
    char *fileData;

    if(argc != 3) {
        printf("Usage: %s [port] [file name]\n", argv[0]);
        exit(1);
    }

    printf("Waiting on port %s to send file \"%s\"...\n", argv[1], argv[2]);

    if((filePointer = fopen(argv[2], "r")) == NULL) {
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
    
    if((connectionSocketDescriptor = listenFromClient(atoi(argv[1]))) == -1) {
        perror("Error: Listening socket ready failed");
        exit(1);
    }

    if(sendToClient(connectionSocketDescriptor, argv[2], strlen(argv[2])+1) == -1) {
        perror("Error: File name sending failed");
        exit(1);
    }

    if(sendToClient(connectionSocketDescriptor, fileSizeString, strlen(fileSizeString)+1) == -1) {
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