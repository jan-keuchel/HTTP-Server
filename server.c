#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUFSIZE 500

char *handle_request(char *req) {

    // Initialize string tokenization
    char *token = strtok(req, " ");

    if (token == NULL) {
        fprintf(stderr, "strtok error: given empty string.\n");
        return NULL;
    }

    // Check if a HTTP GET request was received
    if (strcmp(token, "GET") != 0) {
        fprintf(stderr, "Did not get a 'GET' request. We don't do that here...\n");
        return NULL;
    }

    token = strtok(NULL, " ");
    if (token == NULL) {
        fprintf(stderr, "strtok error: No resource specified.\n");
        return NULL;
    }

    // Create the HTTP response
    char msg[] = "HTTP/1.1 200 OK\r\n\r\nYou just requested the reousece: ";
    char *ret = malloc(strlen(msg) + strlen(token) + 1);
    strcpy(ret, msg);
    ret = strcat(ret, token);
    if (ret == NULL) {
        fprintf(stderr, "strdup error: %s\n", strerror(errno));
        return NULL;
    }

    return ret;
}

int main(void) {
    printf("--- Server ---\n");

    int             s;
    int             sfd;
    struct addrinfo hints;
    struct addrinfo *res, *rp;

    // Set everything to 0
    memset(&hints, 0, sizeof(hints));

    hints.ai_family     = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype   = SOCK_STREAM;  // TCP
    hints.ai_flags      = AI_PASSIVE;   // We want to bind the address
    hints.ai_protocol   = 0;            // any protocol

    // Creating the addrinfo structure
    s = getaddrinfo(NULL, "2390", &hints, &res); // Localhost on port 2390
    if (s != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(errno));
        exit(1);
    }

    // Additional check
    if (res == NULL) {
        fprintf(stderr, "resulting addrinfo is NULL.\n");
        exit(1);
    }

    // Find the socket that can be bound
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        // Get a file descriptor through clients can be accepted
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        // bind the socket file descriptor to an address
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    // Free heap-allocated data
    freeaddrinfo(res);

    // Check if none of the addrinfo's worked
    if (rp == NULL) {
        fprintf(stderr, "Could not bind.\n");
        exit(EXIT_FAILURE);
    }

    // Mark socket sfd as passive: a socket that is used to 
    // accept incoming clonnection requests
    if (listen(sfd, 10) == -1) {
        fprintf(stderr, "listen error: %s\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_storage c_addr;
    socklen_t               c_addr_len = sizeof(c_addr);

    for (;;) {
        printf("Waiting for client...\n");

        // Accept in incoming connection request
        // This yields a new file descriptor for the specific client
        int cfd = accept(sfd, (struct sockaddr *) &c_addr, &c_addr_len);
        if (cfd == -1) {
            fprintf(stderr, "accept error: %s\n", strerror(errno));
            exit(1);
        }

        printf("Client connected.\n");
        printf("Waiting for HTTP request...\n");

        char buf[BUFSIZE] = {0};
        int n = 0;

        // --- Read data
        if (n = recv(cfd, buf, BUFSIZE, 0), n == -1) {
            fprintf(stderr, "recv error: %s\n", strerror(errno));
            exit(1);
        }

        if (n == 0) {
            printf("Client termianted connection.\n");
            continue;
        }

        // Print request for debugging
        printf("Received from client:\n%s\n", buf);

        // Terminate string after first line
        for (int j = 0; j < n; j++) {
            if (buf[j] == '\n')
                buf[j] = '\0';
        }

        printf("Extracted first line:\n%s\n", buf);

        char *res = handle_request(buf);

        // --- Send response
        n = send(cfd, res, strlen(res), 0);
        if (n != strlen(res)) {
            fprintf(stderr, "send error: %s\n", strerror(errno));
            fprintf(stderr, "Only sent %d bytes.\n", n);
            exit(1);
        }

        free(res);
        close(cfd);
    }

    close(sfd);

    exit(EXIT_SUCCESS);
}
