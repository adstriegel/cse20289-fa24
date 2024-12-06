/* http_server.c: simple HTTP server */

/* Source: 

Prof. Bui's System Programming Class - Spring 2023

https://github.com/nd-cse-20289-sp23/cse-20289-sp23-examples/blob/master/lecture21/http_server.c

*/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/* Constants */
#define WHITESPACE      " \t\n"

/* Macros */
#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...)   fprintf(stderr, "[%5d] DEBUG %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)
#endif
#define log(M, ...)     fprintf(stderr, "[%5d] LOG   %10s:%-4d " M "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__)

/* Allocate a socket, bind it to port, and start listening */
int socket_listen(const char *port) {
    /* Lookup server address information */
    struct addrinfo hints = {
        .ai_family      = AF_UNSPEC,    /* Use either IPv4 or IPv6 */
        .ai_socktype    = SOCK_STREAM,  /* Use TCP */
        .ai_flags       = AI_PASSIVE,   /* Use all interfaces to listen */
    };
    struct addrinfo *results;
    int status;
    if ((status = getaddrinfo(NULL, port, &hints, &results)) != 0)  {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        return -1;
    }

    /* For each address entry, allocate socket, bind, and listen */
    int server_fd = -1;
    for (struct addrinfo *p = results; p && server_fd < 0; p = p->ai_next) {
        /* Allocate socket */
        if ((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            fprintf(stderr, "socket failed: %s\n", strerror(errno));
            continue;
        }

        /* Bind socket to port */
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) < 0) {
            fprintf(stderr, "bind failed: %s\n", strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }

        /* Listen on socket */
        if (listen(server_fd, SOMAXCONN) < 0) {
            fprintf(stderr, "listen failed: %s\n", strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
    }
    freeaddrinfo(results);

    return server_fd;
}

/* Accept an incoming client and return client stream */
FILE *accept_client(int server_fd) {
    /* Accept incoming connection */
    struct sockaddr client_addr;
    socklen_t       client_len = sizeof(struct sockaddr);

    int client_fd = accept(server_fd, &client_addr, &client_len);
    if (client_fd < 0) {
        fprintf(stderr, "accept failed: %s\n", strerror(errno));
        return NULL;
    }

    /* Open a file stream from socket file descriptor */
    FILE *client_file = fdopen(client_fd, "w+");
    if (!client_file) {
        fprintf(stderr, "fdopen failed: %s\n", strerror(errno));
        close(client_fd);
    }

    return client_file;
}

/* Handle request */
void handle_request(FILE *client_file) {
    char buffer[BUFSIZ];

    /* Parse URI */
    if (!fgets(buffer, BUFSIZ, client_file)) {
        debug("fgets failed");
        return;
    }

    char *method = strtok(buffer, WHITESPACE);
    char *uri    = strtok(NULL, WHITESPACE);

    debug("METHOD = %s", method);
    debug("URI    = %s", uri);

    /* Skip Headers */
    while (fgets(buffer, BUFSIZ, client_file) && strlen(buffer) > 2);
    
    /* Return message */
    fprintf(client_file, "HTTP/1.0 200 OK\n");
    fprintf(client_file, "Content-Type: text/html\n");
    fprintf(client_file, "\r\n");
    fprintf(client_file, "<h1>Wake me up inside (I can't wake up)</h1>");

    return;
}

/* Main Execution */

int main(int argc, char *argv[]) {
    char *port = argv[1];

    /* Listen to socket */
    int server_fd = socket_listen(port);
    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    debug("Listening on port: %s", port);

    /* Process incoming connections */
    while (1) {
        /* Accept an incoming connection */
        debug("Accepting client request");
        FILE *client_file = accept_client(server_fd);
        if (!client_file)
            continue;
    
        /* Handle client request */
        debug("Handling client request");
        handle_request(client_file);
        fclose(client_file);
    }

    return EXIT_SUCCESS;
}

/* vim: set sts=4 sw=4 ts=8 expandtab ft=c: */