#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

int socket_listen(const char *port);
void usage(char *filename);
void signalHandler(int signum);

int main(int argc, char *argv[]) {
    // Register signals and handlers
    signal(SIGQUIT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Process command-line args
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *port = argv[1];
    std::string dir = argv[2];

    // create a socket using TCP IP
    int sockfd = socket_listen(port);

    int connections = 0;
    while (true) {
        // accept a new connection from a client
        struct sockaddr clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, &clientAddr, &clientAddrSize);

        if (clientSockfd == -1) {
            fprintf(stderr, "ERROR: accept: %s\n", strerror(errno));
            close(sockfd);
            return EXIT_FAILURE;
        }
        connections++;

        // open file to be written
        std::string path = dir + "/" + std::to_string(connections) + ".file";
        int filefd = open(path.c_str(), (O_WRONLY | O_CREAT));
        if (filefd == -1) {
            fprintf(stderr, "ERROR: open: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        // receive file from the client
        char buf[1024];
        while (true) {
            // TODO: Somewhere here error if no data received for over 10 seconds.

            // Receive buf from socket
            ssize_t recvlen = recv(clientSockfd, buf, sizeof(buf), 0);
            if (recvlen == -1) {
                fprintf(stderr, "ERROR: recv: %s\n", strerror(errno));
                close(filefd);
                close(clientSockfd);
                close(sockfd);
                return EXIT_FAILURE;
            } else if (recvlen == 0) {
                break;
            } else {
                // Write buf to file
                ssize_t writelen = write(filefd, buf, recvlen);
                if (writelen == -1) {
                    fprintf(stderr, "ERROR: write: %s\n", strerror(errno));
                    close(filefd);
                    close(clientSockfd);
                    close(sockfd);
                    return EXIT_FAILURE;
                }
            }
        }

        close(filefd);
        close(clientSockfd);
    }

    close(sockfd);

    return EXIT_SUCCESS;
}

// Partly based on socket_dial
/**
 * Create listening socket at localhost in the specified port.
 *
 * @param   port        Port string to bind to.
 *
 * @return  Socket file descriptor of listening socket if successful, otherwise -1.
 **/
int socket_listen(const char *port) {
    const char *host = "localhost";

    /* Lookup server address information */
    struct addrinfo hints = {
        .ai_flags = (AI_V4MAPPED_CFG | AI_ADDRCONFIG | AI_PASSIVE),  // Use default flags and config socket for bind
        .ai_family = AF_UNSPEC,                                      // Any address family (IPv4 or IPv6)
        .ai_socktype = SOCK_STREAM,                                  // Full-duplex byte stream (TCP)
        .ai_protocol = IPPROTO_TCP                                   // TCP protocol
    };
    struct addrinfo *results;

    int status = getaddrinfo(host, port, &hints, &results);  // Dynamic allocation to results
    if (status != 0) {
        fprintf(stderr, "ERROR: getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    /* For each server entry, allocate socket and try to listen */
    int socket_fd = -1;
    for (struct addrinfo *p = results; p && (socket_fd == -1); p = p->ai_next) {
        /* Allocate socket */
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            // fprintf(stderr, "ERROR: socket: %s\n", strerror(errno));
            continue;
        }

        // allow other to reuse the address
        int yes = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            close(socket_fd);
            socket_fd = -1;
            continue;
        }

        /* Bind socket */
        if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            socket_fd = -1;
            continue;
        }

        /* Listen to socket */
        if (listen(socket_fd, 1) == -1) {
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

    fprintf(stderr, "Usage: %s <PORT> <FILE-DIR>\n", filename);
    fprintf(stderr, "    <PORT>: port number on which server will listen on connections. The server must accept connections coming from any interface.\n");
    fprintf(stderr, "    <FILE-DIR>: directory name where to save the received files.\n");
}

void signalHandler(int signum) {
    exit(EXIT_SUCCESS);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=cpp: */