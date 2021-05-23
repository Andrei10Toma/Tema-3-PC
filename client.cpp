#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <iostream>

using namespace std;
using namespace nlohmann;

// TODO: remove char * and replace it with strings because it looks fucking
// nicer

const char *host = "34.118.48.238";
const char *url_register= "/api/v1/tema/auth/register";
const char *content_type = "application/json";
const char *url_login = "/api/v1/tema/auth/login";
const char *url_access = "/api/v1/tema/library/access";

void send_register_command(int sockfd) {
    json register_json;
    string username;
    string password;
    cout << "username="; getline(cin, username);
    cout << "password="; getline(cin, password);
    register_json["username"] = username;
    register_json["password"] = password;
    char *send_json = strdup(register_json.dump().c_str());
    const char **body_data = (const char **)calloc(1, sizeof(char *));
    body_data[0] = send_json;
    // create the request message
    char *message = compute_post_request(host, url_register, content_type,
        body_data, 1, NULL, 0);
    send_to_server(sockfd, message);
    // receive the response from the server
    char *response = receive_from_server(sockfd);
    // extract the body from the response
    char *body = basic_extract_json_response(response);
    // print succes message or error message
    if (body == NULL) {
        cout << "Welcome to the nerd club!" << endl;
    } else {
        cout << body;
    }
    // free allocated memory
    free(send_json);
    free(body_data);
}

char *extract_cookie(char *response) {
    char *set_cookie = strstr(response, "Set-Cookie: ");
    char *token = strtok(set_cookie, DELIM);
    token += strlen("Set-Cookie: ");
    return token;
}

string send_login_command(int sockfd) {
    json login_json;
    string username;
    string password;
    cout << "username="; getline(cin, username);
    cout << "password="; getline(cin, password);
    login_json["username"] = username;
    login_json["password"] = password;
    char *send_json = strdup(login_json.dump().c_str());
    const char **body_data = (const char **)calloc(1, sizeof(char *));
    // create the json that will be sent
    body_data[0] = send_json;
    // create the request message
    char *message = compute_post_request(host, url_login, content_type, 
        body_data, 1, NULL, 0);
    // send the message to the server
    send_to_server(sockfd, message);
    // receive the response from the server
    char *response = receive_from_server(sockfd);
    char *body = basic_extract_json_response(response);
    // print succes message or error messaage
    if (body == NULL) {
        cout << "You managed to login! Here is a cookie for you!" << endl;
    } else {
        cout << body << endl;
    }
    // free allocated memory
    free(send_json);
    free(body_data);
    // extract the cookie from the received message
    string cookie = extract_cookie(response);
    return cookie;
}

// TODO: after replacing char* with string do this function
void send_enter_library_command(int sockfd, string cookie) {
}

int main(int argc, char *argv[]) {
    char command[15];
    int sockfd;
    string cookie;
    // receive commands and interpret them
    while (true) {
        // open the connection with the server
        sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
        memset(command, 0, 15);
        fgets(command, 15, stdin);
        // remove \n from the end of the string
        command[strlen(command) - 1] = '\0';
        if (strcmp(command, "register") == 0) {
            send_register_command(sockfd);
        } else if (strcmp(command, "login") == 0) {
            cookie = send_login_command(sockfd);
        } else if (strcmp(command, "enter_library") == 0) {
            send_enter_library_command(sockfd, cookie);
        } else if (strcmp(command, "get_books") == 0) {

        } else if (strcmp(command, "get_book") == 0) {

        } else if (strcmp(command, "add_book") == 0) {

        } else if (strcmp(command, "delete_book") == 0) {

        } else if (strcmp(command, "logout") == 0) {

        } else if (strcmp(command, "exit") == 0) {
            break;
        }
        close_connection(sockfd);
    }

    // close the connection
    return 0;
}
