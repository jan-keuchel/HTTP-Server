#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSIZE 500

int main(void) {
    printf("--- Client ---\n");

    int             s;
    int             sfd;
    struct addrinfo hints;
    struct addrinfo *res, *rp;

    // Set everything to 0
    memset(&hints, 0, sizeof(hints));
 
    // And specify some values
    hints.ai_family     = AF_INET;      // Use IPv4
    hints.ai_socktype   = SOCK_STREAM;  // Reliable messages
    hints.ai_protocol   = 0;            // Use any protocol

    // Creating the addrinfo structure
    s = getaddrinfo("localhost", "2390", &hints, &res);
    if (s != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(errno));
        exit(1);
    }

    // Additional check
    if (res == NULL) {
        fprintf(stderr, "resulting addrinfo is NULL.\n");
        exit(1);
    }

    // Find the first connection that works
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        // Get a file descriptor through which can be communicated
        sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sfd == -1)
            continue;

        // Try and connect to the server
        if (connect(sfd, res->ai_addr, res->ai_addrlen) != -1)
            break;

        close(sfd);
    }
    // Free heap-allocated data
    freeaddrinfo(res);

    // Check if none of the addrinfo's worked
    if (rp == NULL) {
        fprintf(stderr, "Could not connect.\n");
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE] = {0};
    char msg[BUFSIZE] = {0};

    for (;;) {
        // --- Read from stdin:
        fgets(msg, BUFSIZE, stdin);

        // -1 because of newline char
        if (strlen(msg) - 1 == 0) {
            printf("End of transmission.\n");
            break;
        }

        // Last char is newline
        msg[strlen(msg) - 1] = '\0';

        // --- Send message to server:
        int n = send(sfd, msg, strlen(msg), 0);
        if (n != strlen(msg)) {
            fprintf(stderr, "send error: %s\n", strerror(errno));
            exit(1);
        }

        // --- Receive response:
        if (n = recv(sfd, buf, BUFSIZE, 0), n == -1) {
            fprintf(stderr, "recv error: %s\n", strerror(errno));
            exit(1);
        }
        buf[n] = '\0';
        printf("Server response: %s\n", buf);
    }



    // Close file descriptor
    close(sfd);

    printf("Success.\n");
    exit(EXIT_SUCCESS);
}
