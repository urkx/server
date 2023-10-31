#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT 8080

void error(char *msg) {
    perror(msg);
    exit(1);
}

bool writeDataToClient(int sckt, const void *data, int datalen) {
    const char *pdata = (const char *)data;

    while(datalen > 0) {
        int numSent = send(sckt, pdata, datalen, 0);
        if(numSent <= 0) {
            if(numSent == 0) {
                printf("Client disconnected\n");
            } else {
                perror("Client not written");
            }
            return false;
        }
        pdata += numSent;
        datalen -= numSent;
    }

    return true;
}

bool writeStrToClient(int sckt, const char *str) {
    return writeDataToClient(sckt, str, strlen(str));
}

bool addHeader(int *socket, char *header) {
    bool res = true;
    if(!writeStrToClient(*socket, header)) {
        close(*socket);
        res = false;
    }
    return res;
}

void handleConnections(int *create_socket, struct sockaddr_in *address, char *buffer, int bufsize, long fsize, char *msg) {
    socklen_t addrlen;
    int new_socket;
    while(1) {
        addrlen = sizeof(*address);
        new_socket = accept(*create_socket, (struct sockaddr *)address, &addrlen);

        if(new_socket == -1) error("Client not accepted");

        printf("Client connected from %s:%hu\n", inet_ntoa(address->sin_addr), ntohs(address->sin_port));

        int numRead = recv(new_socket, buffer, bufsize, 0);
        if(numRead < 1) {
            if(numRead == 0) {
                printf("Client disconnected\n");
            } else {
                perror("Client not read");
            }
            close(new_socket);
            continue;
        }
        printf("%.*s\n", numRead, buffer);

        if(!addHeader(&new_socket, "HTTP/1.1 200 OK\r\n")) continue;

        char clen[40];
        sprintf(clen, "Content-length: %ld\r\n", fsize);

        if(!addHeader(&new_socket, clen)) continue;

        if(!addHeader(&new_socket, "Content-Type: text/html\r\n")) continue;

        if(!addHeader(&new_socket, "Connection: close\r\n\r\n")) continue;

        if(!writeDataToClient(new_socket, msg, fsize)) {
            close(new_socket);
            continue;
        }

        printf("File sent\n");
        close(new_socket);
    }
}

int main() {
    int create_socket;
    char *buffer;
    int bufsize = 1024;
    struct sockaddr_in address;
    buffer = (char *)malloc(bufsize);
    if(!buffer) error("Buffer was not allocated\n");

    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(create_socket == -1) error("Socket not created");
    
    printf("Socket created\n");

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1) error("Socket not bound");

    printf("Socket bound\n");

    long fsize;
    FILE *fp = fopen("index.html", "rb");
    if(!fp) error("File not opened");

    printf("File opened\n");

    if(fseek(fp, 0, SEEK_END) == -1) error("File not seeked");

    fsize = ftell(fp);
    if(fsize == -1) error("File size not retrieved");

    rewind(fp);

    char *msg = (char *)malloc(fsize);
    if(!msg) error("File buffer not allocated");

    if(fread(msg, fsize, 1, fp) != 1) error("File not read");

    fclose(fp);

    if(listen(create_socket, 10) == -1) error("Socket was not opened for listening");

    printf("Socket is listening\n");

    handleConnections(&create_socket, &address, buffer, bufsize, fsize, msg);

    close(create_socket);
    return 0;
}