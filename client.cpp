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

const char *host = "34.118.48.238";
const char *url_register= "/api/v1/tema/auth/register";
const char *content_type = "application/json";
const char *url_login = "/api/v1/tema/auth/login";
const char *url_access = "/api/v1/tema/library/access";
const char *url_books = "/api/v1/tema/library/books";
const char *url_logout = "/api/v1/tema/auth/logout";

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
        body_data, 1, NULL, 0, NULL);
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

char* send_login_command(int sockfd) {
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
        body_data, 1, NULL, 0, NULL);
    // send the message to the server
    send_to_server(sockfd, message);
    // receive the response from the server
    char *response = receive_from_server(sockfd);
    char *body = basic_extract_json_response(response);
    // print succes message or error messaage
    if (body == NULL) {
        free(send_json);
        free(body_data);
        if (extract_cookie(response) == NULL) { 
            return NULL;
        }
        char* cookie = strdup(extract_cookie(response));
        cout << "You managed to login! Here is a cookie for you!" << endl;
        return cookie;
    } else {
        cout << body << endl;
        // free allocated memory
        free(send_json);
        free(body_data);
        return NULL;
    }
}

char *send_enter_library_command(int sockfd, char* cookie) {
    const char **cookies = (const char **)calloc(1, sizeof(char *));
    cookies[0] = cookie;
    char *message = compute_get_request(host, url_access, NULL, 
        cookies, 1, NULL, "GET");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // extract the response from the json
    char *jwt_token_body = basic_extract_json_response(response);
    free(cookies);
    if (jwt_token_body != NULL) {
        json json_token = json::parse(jwt_token_body);
        if (!json_token["token"].is_null()) {
            // extract the token from the response
            char *jwt_token = strdup(json_token["token"].dump().c_str());
            jwt_token++;
            memset(jwt_token + strlen(jwt_token) - 1, 0, 1);
            printf("Welcome in the unknown world of the library. "
                "Here is your token.\n");
            return jwt_token;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

void send_get_books_command(int sockfd, char *jwt_token) {
    char *message = compute_get_request(host, url_books, NULL, 
        NULL, 0, jwt_token, "GET");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // extract the body from the
    char *body = strstr(response, "[{");
    if (body != NULL) {
        printf("%s\n", body);
    }
    else {
        printf("We dont have books here.\n");
    }
}

void send_add_book_command(int sockfd, char *jwt_token) {
    string title, author, genre, publisher, page_count;
    json json_add_book;
    cout << "title="; getline(cin, title);
    cout << "author="; getline(cin, author);
    cout << "genre="; getline(cin, genre);
    cout << "page_count="; getline(cin, page_count);
    cout << "publisher="; getline(cin, publisher);
    // create the json that will be sent to the server
    json_add_book["title"] = title;
    json_add_book["author"] = author;
    json_add_book["genre"] = genre;
    json_add_book["page_count"] = page_count;
    json_add_book["publisher"] = publisher;
    char *send_json = strdup(json_add_book.dump().c_str());
    const char **body_data = (const char **)calloc(1, sizeof(char *));
    body_data[0] = send_json;
    char *message = compute_post_request(host, url_books, content_type, 
        body_data, 1, NULL, 0, jwt_token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *body = basic_extract_json_response(response);
    if (body == NULL) {
        printf("The book was added to the library.\n");
    } else  {
        // error
        printf("%s\n", body);
    }
    // free the allocated memory
    free(send_json);
    free(body_data);
}

void send_get_book_command(int sockfd, char *jwt_token) {
    string id;
    cout << "id="; getline(cin, id);
    // create the message to get the book with the given id
    string url_get_book = url_books;
    url_get_book.append("/").append(id);
    char *message = compute_get_request(host, url_get_book.c_str(), NULL, NULL,
        0, jwt_token, "GET");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *body = strstr(response, "[{");
    char *body_error = basic_extract_json_response(response);
    if (body != NULL) {
        body++;
        memset(body + strlen(body) - 1, 0, 1);
        json book_information = json::parse(body);
        printf("Informations about the book you asked for:\n");
        cout << "title=" << book_information["title"] << endl << "author="
            << book_information["author"] << endl << "genre="
            << book_information["genre"] << endl << "pages="
            << book_information["page_count"] << endl << "publisher="
            << book_information["publisher"] << endl;
    } else {
        printf("%s\n", body_error);
    }
}

void send_delete_book_command(int sockfd, char *jwt_token) {
    string id;
    cout << "id="; getline(cin, id);
    // create the message to delete the book with the given id
    string url_delete_book = url_books;
    url_delete_book.append("/").append(id);
    char *message = compute_get_request(host, url_delete_book.c_str(), NULL,
        NULL, 0, jwt_token, "DELETE");
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    char *body = basic_extract_json_response(response);
    if (body == NULL) {
        printf("Book deleted.\n");
    } else {
        printf("%s\n", body);
    }
}

void send_logout_command(int sockfd, char *cookie) {
    const char **cookies = (const char **)calloc(1, sizeof(char *));
    cookies[0] = cookie;
    char *message = compute_get_request(host, url_logout, NULL, 
        cookies, 1, NULL, "GET");
    send_to_server(sockfd, message);
    char *receive = receive_from_server(sockfd);
    char *body = basic_extract_json_response(receive);
    if (body == NULL) {
        printf("Logged out succesfully! Bye!\n");
    } else {
        printf("%s\n", body);
    }
}

int main(int argc, char *argv[]) {
    char command[15];
    int sockfd;
    char *cookie = NULL, *jwt_token = NULL;
    // receive commands and interpret them
    while (true) {
        // open the connection with the server
        sockfd = open_connection(host, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error("Stupid socket.\n");
            break;
        }
        memset(command, 0, 15);
        fgets(command, 15, stdin);
        // remove \n from the end of the command string 
        memset(command + strlen(command), 0, 1);
        command[strlen(command) - 1] = '\0';
        if (strcmp(command, "register") == 0) {
            send_register_command(sockfd);
        } else if (strcmp(command, "login") == 0) {
            if (cookie == NULL) {
                cookie = send_login_command(sockfd);
            } else {
                printf("You must logout first to login to another account.\n");
            }
        } else if (strcmp(command, "enter_library") == 0) {
            if (cookie != NULL) {
                jwt_token = send_enter_library_command(sockfd, cookie);
            } else {
                printf("You must login first.\n");
            }
        } else if (strcmp(command, "get_books") == 0) {
            if (jwt_token != NULL) {
                send_get_books_command(sockfd, jwt_token);
            } else {
                printf("You must have access to the library first.\n");
            }
        } else if (strcmp(command, "get_book") == 0) {
            if (jwt_token != NULL) {
                send_get_book_command(sockfd, jwt_token);
            } else {
                printf("You must have access to the library first.\n");
            }
        } else if (strcmp(command, "add_book") == 0) {
            if (jwt_token != NULL) {
                send_add_book_command(sockfd, jwt_token);
            } else {
                printf("You must have access to the library frist.\n");
            }
        } else if (strcmp(command, "delete_book") == 0) {
            if (jwt_token != NULL) {
                send_delete_book_command(sockfd, jwt_token);
            } else {
                printf("You must have access to the library first.\n");
            }
        } else if (strcmp(command, "logout") == 0) {
            if (cookie == NULL) {
                printf("You can't logout if you are not logged in. DUH!\n");
            } else {
                send_logout_command(sockfd, cookie);
                jwt_token = NULL;
                cookie = NULL;
            }
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Wrong command!\n");
        }
        close_connection(sockfd);
    }
    // close the connection
    close_connection(sockfd);
    return 0;
}
