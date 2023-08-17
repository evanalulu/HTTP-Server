#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "network.h"

#define PORT "8100"

#define MAX_HTTP_LEN 8192
#define MAX_PATH_LEN 256
#define CONTENT_TYPE_LEN 32

int create_tcp_socket(const char *port) {
    // Creating TCP socket connection with local host
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *results;
    int status = getaddrinfo(NULL, port, &hints, &results);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Binding socket to the port
    int sockfd = -1;
    for (struct addrinfo *p = results; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket error");
            continue;
        }

        int confirm = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &confirm, sizeof(confirm)) == -1) {
            perror("setsockopt error");
            close(sockfd);
            continue;
        }

        if (ioctl(sockfd, FIONBIO, &confirm) == -1) {
            perror("ioctl error");
            close(sockfd);
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind error");
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(results);

    if (sockfd == -1) {
        fprintf(stderr, "Failed to create and bind socket\n");
        exit(EXIT_FAILURE);
    }

    // Listening for TCP connections
    if (listen(sockfd, SOMAXCONN) == -1) {
        perror("Failed to listen for connections\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void handle_new_connection(int new_fd) {
    // Receiving and handling the message
    struct pollfd pfd[1];
    pfd[0].fd = new_fd;
    pfd[0].events = POLLIN;

    int res = poll(pfd, 1, 5000);
    if (res < 0) {
        perror("poll() error");
        exit(0);
    }

    char file_path[MAX_PATH_LEN + 1];
    char http_req[MAX_HTTP_LEN];
    memset(http_req, 0, MAX_HTTP_LEN);
    read(new_fd, http_req, MAX_HTTP_LEN);

    parse_req(file_path, http_req);

    char relative_path[MAX_PATH_LEN] = "www/cs221.cs.usfca.edu";
    strcat(relative_path, file_path);
    if (!strcmp(file_path, "/"))
        strcat(relative_path, "index.html");

    char content_type[33];
    memset(content_type, 0, 33);

    switch (get_content_type(relative_path)) {
        case 0:
            strncpy(content_type, "text/html", CONTENT_TYPE_LEN);
            break;
        case 1:
            strncpy(content_type, "text/css", CONTENT_TYPE_LEN);
            break;
        case 2:
            strncpy(content_type, "image/jpeg", CONTENT_TYPE_LEN);
            break;
        case 3:
            strncpy(content_type, "image/png", CONTENT_TYPE_LEN);
            break;
    }

    FILE *fp = fopen(relative_path, "rb"); // open file in binary mode
    if (!fp) {
        send_response(new_fd, "404 Not Found", "text/html",
                      "<!DOCTYPE html>\n"
                      "<html>\n"
                      "  <body>\n"
                      "    Not found\n"
                      "  </body>\n"
                      "</html>\n", 65);
        close(new_fd);
        return;
    }

    // Calculating file size
    fseek(fp, 0L, SEEK_END);
    long int file_size = ftell(fp);
    rewind(fp);

    char *file_data = malloc(file_size);
    fread(file_data, file_size, 1, fp); // read raw bytes of file data

    send_response(new_fd, "200 OK", content_type, file_data, file_size);

    free(file_data);
    fclose(fp);
    close(new_fd);
}

