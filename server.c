#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>


int main(void) {
    printf("--- Server ---\n");

    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family     = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype   = SOCK_STREAM;  // TCP
    hints.ai_flags      = AI_PASSIVE;   // wildcard IP address
    hints.ai_protocol   = 0;            // any protocol
    int s = getaddrinfo(NULL, "2390", &hints, &res);
    if (s != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(errno));
        exit(1);
    }

    if (res == NULL) {
        fprintf(stderr, "resulting addrinfo is NULL.\n");
        exit(1);
    }

    // socket(domain, type, protocol)
    // Create a socket (file descriptor)
    // domain   -> AF_INET for IPv4
    // type     -> SOCK_STREAM for TCP or SOCK_DGRAM for UDP
    // protocol -> 0
    // Returns fd on success or -1 on failure and sets errno
    int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        fprintf(stderr, "sfd error: %s\n", strerror(errno));
        exit(1);
    }

    // bind the socket file descriptor to an address
    if (bind(sfd, res->ai_addr, res->ai_addrlen) != 0) {
        fprintf(stderr, "bind error: %s\n", strerror(errno));
        exit(1);
    }

    freeaddrinfo(res);

    if (listen(sfd, 10) == -1) {
        fprintf(stderr, "listen error: %s\n", strerror(errno));
        exit(1);
    }


    struct sockaddr_storage c_addr;
    socklen_t               c_addr_len = sizeof(c_addr);
    int cfd = accept(sfd, (struct sockaddr *) &c_addr, &c_addr_len);
    if (cfd == -1) {
        fprintf(stderr, "accept error: %s\n", strerror(errno));
        exit(1);
    }

    char buf[500] = {0};
    if (recv(cfd, buf, 500, 0) == -1) {
        fprintf(stderr, "recv error: %s\n", strerror(errno));
        exit(1);
    }

    printf("Server received: %s\n", buf);

    close(cfd);
    close(sfd);


}
