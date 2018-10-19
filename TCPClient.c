#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#define FILE_NAME_SIZE 128

int receiveFromServer(char *data, int data_size, char *addr, int port) {
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
    if(recv(sockfd, data, data_size, 0) < 0) {
        perror("Error: Data receiving failed");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    FILE *filePointer;
    char fileName[FILE_NAME_SIZE];
    char fileSizeBuffer[13];
    int fileSize;
    char *fileData;

    if(argc != 3) {
        printf("Usage: %s [server address] [port]\n", argv[0]);
        exit(1);
    }

    printf("Connecting to %s:%s...\n", argv[1], argv[2]);

    if(receiveFromServer(fileName, sizeof(fileName), argv[1], atoi(argv[2])) == -1) {
        perror("Error: File name receiving failed");
        exit(1);
    }

    printf("Receiving file \"%s\"...\n", fileName);

    if((filePointer = fopen(fileName, "w")) == NULL) {
        perror("Error: File accessing failed");
        exit(1);
    }

    if(receiveFromServer(fileSizeBuffer, sizeof(fileSizeBuffer), argv[1], atoi(argv[2])) == -1) {
        perror("Error: File size receiving failed");
        exit(1);
    }

    fileSize = atoi(fileSizeBuffer);
    fileData = malloc(fileSize);

    if(receiveFromServer(fileData, fileSize, argv[1], atoi(argv[2])) == -1) {
        perror("Error: File data receiving failed");
        exit(1);
    }

    if((fileSize = fwrite(fileData, 1, fileSize, filePointer)) < 0) {
        perror("Error: File writing failed");
        exit(1);
    }

    fclose(filePointer);

    printf("Receiving complete.\n");

    return 0;
}