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

// create a message for "GET" or a "DELETE" request, depending on the 
// request parameter
char *compute_get_request(const char *host, const char *url, 
    const char **cookies, int cookies_count, const char *token,
     const char *request) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    sprintf(line, "%s %s HTTP/1.1", request, url);

    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // insert the cookies in the message
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            memset(line, 0, LINELEN);
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // add the authorization token
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}

// create a message for the "POST" request
char *compute_post_request(const char *host, const char *url,
    const char* content_type, const char **body_data, 
    int body_data_fields_count, const char *token) {
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
    // every time in body data will be the json field
    // calculate the length of the content
    for (int i = 0; i < body_data_fields_count; i++) {
        size += strlen(body_data[i]);
        strcat(body_data_buffer, body_data[i]);
    }
    sprintf(line, "Content-Length: %d", size);
    compute_message(message, line);
    // put the token
    if (token != NULL) {
        memset(line, 0, LINELEN);
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");
    compute_message(message, body_data_buffer);

    free(line);
    free(body_data_buffer);
    return message;
}
