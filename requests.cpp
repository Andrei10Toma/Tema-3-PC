#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, 
    const char *query_params, const char **cookies, int cookies_count, 
    const char *token, const char *request) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "%s %s?%s HTTP/1.1",request, url, query_params);
    } else {
        sprintf(line, "%s %s HTTP/1.1", request, url);
    }

    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // add the tokens in the request

    compute_message(message, "");
    return message;
}

char *compute_post_request(const char *host, const char *url,
    const char* content_type, const char **body_data, 
    int body_data_fields_count, const char **cookies, int cookies_count,
    const char *token) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char *)calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    memset(line, 0, LINELEN);
    int size = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        size += strlen(body_data[i]);
        strcat(body_data_buffer, body_data[i]);
    }
    sprintf(line, "Content-Length: %d", size);
    compute_message(message, line);

    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    if (cookies != NULL) {

    }
    compute_message(message, "");
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
