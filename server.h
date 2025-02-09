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
#include <pthread.h>
#include "tpool.h"

#define PORT 8080
#define POOL_SIZE 5UL

static char *root = {"/"};

struct Resource {
    char *resource;
    long fsize;
};

typedef struct {
    struct sockaddr_in* address;
    int id;
    pthread_mutex_t* lock;
} Connection;
