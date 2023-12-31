#include "server.h"

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

// Aux function that adds a header to a socket
bool addHeader(int *socket, char *header) {
    bool res = true;
    if(!writeStrToClient(*socket, header)) {
        close(*socket);
        res = false;
    }
    return res;
}


// HTTP 200 Response handler
// Returns true if no errors happened
bool httpOk(struct Resource *resource, int *new_socket) {
    if(!addHeader(new_socket, "HTTP/1.1 200 OK\r\n")) return false;

    char clen[40];
    sprintf(clen, "Content-length: %ld\r\n", resource->fsize);

    if(!addHeader(new_socket, clen)) return false;

    if(!addHeader(new_socket, "Content-Type: text/html\r\n")) return false;

    if(!addHeader(new_socket, "Connection: close\r\n\r\n")) return false;

    if(!writeDataToClient(*new_socket, resource->resource, resource->fsize)) {
        close(*new_socket);
        return false;
    }

    return true;
}

// HTTP 404 Response handler
// Returns true if no errors happened
bool httpNotFound(int *new_socket) {
    if(!addHeader(new_socket, "HTTP/1.1 404 Not Found\r\n")) return false;
    return true;
}

// Aux function that retrieves a file
// Returns a buffer that contains file content and its size
struct Resource getResource(char *buffer) {
    char *fh = strtok(buffer, "\r\n");
    strtok(fh, " ");
    char *resource = strtok(NULL, " ");
    struct Resource res;
    res.fsize = 0L;

    if(strcmp(resource, root) == 0) resource = "/index.html";

    resource = resource + 1;

    FILE *fp = fopen(resource, "rb");
    if(fp != NULL) { 
        printf("File opened\n");

        if(fseek(fp, 0, SEEK_END) == -1) error("File not seeked");

        long fsize = ftell(fp);
        if(fsize == -1) error("File size not retrieved");

        rewind(fp);

        char *msg = (char *)malloc(fsize);
        if(!msg) error("File buffer not allocated");

        if(fread(msg, fsize, 1, fp) != 1) error("File not read");

        res.fsize = fsize;
        res.resource = msg;

        fclose(fp);
    }

    return res;
}

// Handles connections
void handleConnections(int *create_socket, struct sockaddr_in *address) {
    socklen_t addrlen;
    char *buffer;
    int bufsize = 1024;
    int new_socket;
    buffer = (char *)malloc(bufsize);
    if(!buffer) error("Buffer for connection was not allocated\n");

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

        struct Resource resource = getResource(buffer);

        if(resource.fsize == 0L) {
            if(!httpNotFound(&new_socket)) continue;
        } else {
            if(!httpOk(&resource, &new_socket)) continue;
            printf("File sent\n");
            free(resource.resource);
        }

        close(new_socket);
    }

    free(buffer);
}

int main() {
    int create_socket;
    struct sockaddr_in address;

    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(create_socket == -1) error("Socket not created");
    
    printf("Socket created\n");

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1) error("Socket not bound");

    printf("Socket bound\n");

    if(listen(create_socket, 10) == -1) error("Socket was not opened for listening");

    printf("Socket is listening\n");

    handleConnections(&create_socket, &address);

    close(create_socket);
    return 0;
}