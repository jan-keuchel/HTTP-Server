#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>




int main(void) {
    printf("--- Client ---\n");

    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = 0;

    int s = getaddrinfo("localhost", "2390", &hints, &res);
    if (s != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(errno));
        exit(1);
    }

    if (res == NULL) {
        fprintf(stderr, "resulting addrinfo is NULL.\n");
        exit(1);
    }

    int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        fprintf(stderr, "sfd error: %s\n", strerror(errno));
        exit(1);
    }

    if (connect(sfd, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "connect error: %s\n", strerror(errno));
        exit(1);
    }

    char* msg = "Hello there!\n";
    int n = send(sfd, msg, strlen(msg), 0);
    if (n != strlen(msg)) {
        fprintf(stderr, "send error: %s\n", strerror(errno));
        exit(1);
    }

    printf("Success.\n");
}
