#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

int socket_dial(const char *host, const char *port);
void usage(char *filename);

int main(int argc, char *argv[]) {
    // Process command-line args
    if (argc != 4) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *host = argv[1];
    char *port = argv[2];
    char *path = argv[3];

    // Open file
    int filefd = open(path, O_RDONLY);
    if (filefd == -1) {
        fprintf(stderr, "ERROR: open: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // create a socket using TCP IP
    int sockfd = socket_dial(host, port);
    if (sockfd == -1) {
        fprintf(stderr, "ERROR: Failed to create TCP connection.\n");
        return EXIT_FAILURE;
    }

    // send file to the server
    char buf[1024];
    while (true) {
        ssize_t readlen = read(filefd, buf, sizeof(buf));
        if (readlen == -1) {
            fprintf(stderr, "ERROR: read: %s\n", strerror(errno));
            close(sockfd);
            close(filefd);
            return EXIT_FAILURE;
        } else if (readlen == 0) {
            break;
        } else {
            if (send(sockfd, buf, readlen, 0) == -1) {
                fprintf(stderr, "ERROR: send: %s\n", strerror(errno));
                close(sockfd);
                close(filefd);
                return EXIT_FAILURE;
            }
        }
    }

    close(sockfd);
    close(filefd);

    return EXIT_SUCCESS;
}

// Partially from Homework 9 of Systems Programming
/**
 * Create TCP socket connection to specified host and port.
 *
 * @param   host        Host string to connect to.
 * @param   port        Port string to connect to.
 *
 * @return  Socket file descriptor of connection if successful, otherwise -1.
 **/
int socket_dial(const char *host, const char *port) {
    /* Lookup server address information */
    struct addrinfo hints = {
        .ai_flags = (AI_V4MAPPED | AI_ADDRCONFIG),  // Use default flags
        .ai_family = AF_UNSPEC,                     // Any address family (IPv4 or IPv6)
        .ai_socktype = SOCK_STREAM,                 // Full-duplex byte stream (TCP)
        .ai_protocol = IPPROTO_TCP                  // TCP protocol
    };
    struct addrinfo *results;

    int status = getaddrinfo(host, port, &hints, &results);  // Dynamic allocation to results
    if (status != 0) {
        fprintf(stderr, "ERROR: getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    /* For each server entry, allocate socket and try to connect */
    int socket_fd = -1;
    for (struct addrinfo *p = results; p && (socket_fd == -1); p = p->ai_next) {
        /* Allocate socket */
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            // fprintf(stderr, "ERROR: socket: %s\n", strerror(errno));
            continue;
        }

        /* Connect to host */
        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) != 0) {
            // fprintf(stderr, "ERROR: connect: %s\n", strerror(errno));
            close(socket_fd);
            socket_fd = -1;
            continue;
        }
    }

    /* Release allocated address information */
    freeaddrinfo(results);

    if (socket_fd == -1) {
        fprintf(stderr, "ERROR: Unable to connect to %s:%s: %s\n", host, port, strerror(errno));
        return -1;
    }

    return socket_fd;
}

void usage(char *filename) {
    if (!filename)
        return;

    fprintf(stderr, "Usage: %s <HOSTNAME-OR-IP> <PORT> <FILENAME>\n", filename);
    fprintf(stderr, "    <HOSTNAME-OR-IP>: hostname or IP address of the server to connect.\n");
    fprintf(stderr, "    <PORT>: port number of the server to connect.\n");
    fprintf(stderr, "    <FILENAME>: name of the file to transfer to the server after the connection is established.\n");
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=cpp: */
