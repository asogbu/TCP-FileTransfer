#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include <netdb.h>

int socket_dial(const char *host, const char *port);

int
main()
{
  const char *host = "127.0.0.1";
  const char *port = "40000";

  // create a socket using TCP IP
  int sockfd = socket_dial(host, port);
  if (sockfd == -1) {
    fprintf(stderr, "ERROR: Failed to create TCP connection.\n");
    return EXIT_FAILURE;
  }

  // send/receive data (1 message) to/from the server
  bool isEnd = false;
  std::string input;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    std::cout << "send: ";
    std::cin >> input;
    if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ss << buf << std::endl;
    std::cout << "echo: ";
    std::cout << buf << std::endl;

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(sockfd);

  return 0;
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
        .ai_flags = AI_DEFAULT,     // Use default flags
        .ai_family = AF_UNSPEC,     // Any address family (IPv4 or IPv6)
        .ai_socktype = SOCK_STREAM, // Full-duplex byte stream
        .ai_protocol = IPPROTO_TCP  // TCP protocol
    };
    struct addrinfo *results;

    int status = getaddrinfo(host, port, &hints, &results); // Dynamic allocation to results
    if (status != 0) {
        fprintf(stderr, "ERROR: getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    /* For each server entry, allocate socket and try to connect */
    int client_fd = -1;
    for (struct addrinfo *p = results; p && (client_fd == -1); p = p->ai_next) {
        /* Allocate socket */
        client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: socket: %s\n", strerror(errno));
            continue;
        }

        /* Connect to host */
        if (connect(client_fd, p->ai_addr, p->ai_addrlen) != 0) {
            fprintf(stderr, "ERROR: connect: %s\n", strerror(errno));
            close(client_fd);
            client_fd = -1;
            continue;
        }
    }

    /* Release allocated address information */
    freeaddrinfo(results);
    
    if (client_fd == -1) {
        fprintf(stderr, "ERROR: Unable to connect to %s:%s: %s\n", host, port, strerror(errno));
        return -1;
    }

    return client_fd;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=cpp: */