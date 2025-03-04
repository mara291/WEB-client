#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int sockfd;
char *cookie = NULL;
char *token = NULL;

void register_user()
{
    char *message;
    char *response;
    char username[1000], password[1000];

    printf("username=");
    fgets(username, 1000, stdin);
    printf("password=");
    fgets(password, 1000, stdin);
    // check if username contains spaces
    if (strchr(username, ' ') != NULL)
    {
        printf("ERROR: Username should not contain any spaces!\n");
        return;
    }

    // replace newline with null character
    if (strchr(username, '\n'))
        username[strlen(username) - 1] = '\0';
    if (strchr(password, '\n'))
        password[strlen(password) - 1] = '\0';

    // initialise json object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    char *payload = json_serialize_to_string_pretty(root_value);

    // compute message with the correct fields
    message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/register", "application/json", &payload, 0, NULL, 0, NULL);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors or otherwise print success message
    if (strstr(response, "error") != NULL)
        printf("ERROR: Username %s already exists!\n", username);
    else
        printf("SUCCES! User %s succesfully registered!\n", username);

    // free memory
    json_free_serialized_string(payload);
    json_value_free(root_value);
}

void login_user()
{
    char *message;
    char *response;
    char username[1000], password[1000];

    printf("username=");
    fgets(username, 1000, stdin);
    printf("password=");
    fgets(password, 1000, stdin);
    // check if username contains any spaces
    if (strchr(username, ' ') != NULL)
    {
        printf("ERROR: Username should not contain any spaces!\n");
        return;
    }

    // replace endline with null character
    if (strchr(username, '\n'))
        username[strlen(username) - 1] = '\0';
    if (strchr(password, '\n'))
        password[strlen(password) - 1] = '\0';

    // create json object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    char *payload = json_serialize_to_string_pretty(root_value);

    // compute message with the correct fields
    message = compute_post_request("34.246.184.49", "/api/v1/tema/auth/login", "application/json", &payload, 0, NULL, 0, NULL);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print success message and allocate memory for cookie
    if (strstr(response, "error") != NULL)
    {
        printf("ERROR: Credentials don't match!\n");
        return;
    }
    else
    {
        printf("SUCCESS! User logged in!\n");
        char *cookie_start = strstr(response, "Set-Cookie: ");
        if (cookie_start)
        {
            // address of cookie start
            cookie_start += strlen("Set-Cookie: ");
            // address of cookie end
            char *cookie_end = strstr(cookie_start, ";");
            if (cookie_end)
            {
                size_t cookie_length = cookie_end - cookie_start;
                // allocate memory for cookie
                cookie = malloc(cookie_length + 1);
                strncpy(cookie, cookie_start, cookie_length);
                cookie[cookie_length] = '\0';
            }
        }
    }

    // free memory
    json_free_serialized_string(payload);
    json_value_free(root_value);
}

void enter_library()
{
    char *message;
    char *response;

    // check if user is authenticated
    if (cookie == NULL)
    {
        printf("ERROR: User is not authenticated!\n");
        return;
    }

    // compute message with the correct fields
    message = compute_get_request("34.246.184.49", "/api/v1/tema/library/access", NULL, cookie, 1, NULL);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print success message and allocate memory for token
    if (strstr(response, "error") != NULL)
    {
        printf("ERROR: Credentials don't match!\n");
        return;
    }
    else
    {
        printf("SUCCESS! You get access to library!\n");
        char *token_start = strstr(response, "token");
        if (token_start)
        {
            // address of token start
            token_start += strlen("token\":\"");
            // address of token end
            char *token_end = strstr(token_start, "\"");
            if (token_end)
            {
                size_t token_length = token_end - token_start;
                // allocate memory for token
                token = malloc(token_length + 1);
                strncpy(token, token_start, token_length);
                token[token_length] = '\0';
            }
        }
    }
}

void get_books()
{
    char *message;
    char *response;

    // check if there is access to library
    if (token == NULL)
    {
        printf("ERROR: You don't have access to library!\n");
        return;
    }
    else
        printf("You have access to library!\n");

    // compute message with the correct fields
    message = compute_get_request("34.246.184.49", "/api/v1/tema/library/books", NULL, cookie, 1, token);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print the books in the library
    if (strstr(response, "error") != NULL)
        printf("ERROR: You don't have access to library!");
    else
    {
        char *result;
        // address of start of json list
        result = strchr(response, '[');
        JSON_Value *root_value = json_parse_string(result);
        JSON_Array *root_array = json_value_get_array(root_value);

        // check if there are zero books in the library and print message
        if (json_array_get_count(root_array) == 0)
            printf("There are no books in the library!\n");
        // otherwise print books
        else
        {
            for (int i = 0; i < json_array_get_count(root_array); i++)
            {
                JSON_Object *root_object = json_array_get_object(root_array, i);
                const int id = json_object_get_number(root_object, "id");
                const char *title = json_object_get_string(root_object, "title");
                // print each book id and title
                printf("id: %d\ntitle: %s\n\n", id, title);
            }
        }
        // free memory
        json_value_free(root_value);
    }
}

void get_book()
{
    char *message;
    char *response;
    char id[10];
    printf("id=");
    fgets(id, 10, stdin);
    // check if an ID was entered
    if (strlen(id) == 0)
    {
        printf("You did not enter an ID\n");
        return;
    }
    // replace newline with null character
    if (strchr(id, '\n'))
        id[strlen(id) - 1] = '\0';

    // compute the path with the gv=iven ID
    char path[50] = "/api/v1/tema/library/books/";
    strcat(path, id);

    // compute message with the correct fields
    message = compute_get_request("34.246.184.49", path, NULL, cookie, 1, token);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print the book
    if (strstr(response, "error") != NULL)
    {
        if (strstr(response, "Authorization header is missing"))
            printf("ERROR: You do not have access to library!\n");
        if (strstr(response, "id is not int"))
            printf("ERROR: ID is invalid!\n");
        if (strstr(response, "No book was found"))
            printf("ERROR: Book with id %s was not found!\n", id);
        return;
    }
    else
    {
        char *result;
        // address of start of json object
        result = strchr(response, '{');
        JSON_Value *root_value = json_parse_string(result);
        JSON_Object *root_object = json_value_get_object(root_value);

        const char *title = json_object_get_string(root_object, "title");
        const char *author = json_object_get_string(root_object, "author");
        const char *genre = json_object_get_string(root_object, "genre");
        const char *publisher = json_object_get_string(root_object, "publisher");
        const int page_count = json_object_get_number(root_object, "page_count");

        // print book details
        printf("title: %s\n", title);
        printf("author: %s\n", author);
        printf("genre: %s\n", genre);
        printf("publisher: %s\n", publisher);
        printf("page_count: %d\n", page_count);

        // free memory
        json_value_free(root_value);
    }
}

void add_book()
{
    char *message;
    char *response;

    char title[1000];
    char author[100];
    char genre[100];
    char publisher[100];
    char page_count[50];

    // read book details
    printf("title=");
    fgets(title, 1000, stdin);
    printf("author=");
    fgets(author, 100, stdin);
    printf("genre=");
    fgets(genre, 100, stdin);
    printf("publisher=");
    fgets(publisher, 100, stdin);
    printf("page_count=");
    fgets(page_count, 50, stdin);

    // check if all details were completed
    if (strlen(title) == 1 || strlen(author) == 1 || strlen(genre) == 1 || strlen(publisher) == 1 || strlen(page_count) == 1)
    {
        printf("ERROR: You did not complete all fields!\n");
        return;
    }

    // replace newline with endline character
    if (strchr(title, '\n'))
        title[strlen(title) - 1] = '\0';
    if (strchr(author, '\n'))
        author[strlen(author) - 1] = '\0';
    if (strchr(genre, '\n'))
        genre[strlen(genre) - 1] = '\0';
    if (strchr(publisher, '\n'))
        publisher[strlen(publisher) - 1] = '\0';
    if (strchr(page_count, '\n'))
        page_count[strlen(page_count) - 1] = '\0';

    // create json object
    JSON_Value *json_value = json_value_init_object();
    JSON_Object *json_object = json_value_get_object(json_value);
    json_object_set_string(json_object, "title", title);
    json_object_set_string(json_object, "author", author);
    json_object_set_string(json_object, "genre", genre);
    json_object_set_string(json_object, "publisher", publisher);
    json_object_set_string(json_object, "page_count", page_count);

    char *payload = json_serialize_to_string_pretty(json_value);

    // compute message with the correct fields
    message = compute_post_request("34.246.184.49", "/api/v1/tema/library/books", "application/json", &payload, 0, cookie, 1, token);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print success message
    if (strstr(response, "error"))
    {
        if (strstr(response, "Authorization header is missing") != NULL)
            printf("ERROR: You do not have access to library!\n");
        if (strstr(response, "Something Bad Happened") != NULL)
            printf("ERROR: Information is incomplete or was formatted wrong!\n");
        return;
    }
    else
        printf("SUCCESS! Book was added to library!\n");

    // free memory
    json_free_serialized_string(payload);
    json_value_free(json_value);
}

void delete_book()
{
    char *message;
    char *response;
    char id[10];
    printf("id=");
    fgets(id, 10, stdin);
    id[strlen(id) - 1] = '\0';

    // check if an ID was entered
    if (strlen(id) == 0)
    {
        printf("You did not enter an ID\n");
        return;
    }

    // compute path containing the entered ID
    char path[50] = "/api/v1/tema/library/books/";
    strcat(path, id);

    // compute message with the correct fields
    message = compute_delete_request("34.246.184.49", path, token);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print success message
    if (strstr(response, "error") != NULL)
    {
        if (strstr(response, "Authorization header is missing") != NULL)
            printf("ERROR: You do not have access to library!\n");
        if (strstr(response, "Something Bad Happened") != NULL)
            printf("ERROR: ID %s is not valid!\n", id);
    }
    else
        printf("SUCCESS! Book was deleted from library!");
}

void logout_user()
{
    char *message;
    char *response;

    // compute message with the correct fields
    message = compute_get_request("34.246.184.49", "/api/v1/tema/auth/logout", NULL, cookie, 1, token);

    // send message and receive response
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    // check for errors, otherwise print success message
    if (strstr(response, "error") != NULL)
        printf("ERROR: You are not logged in!\n");
    else
        printf("SUCCESS! You are logged out!\n");
    // cookie and token become null
    cookie = NULL;
    token = NULL;
}

int main(int argc, char *argv[])
{
    char input[100];

    // get commands until "exit" command
    while (1)
    {
        fgets(input, 100, stdin);

        sockfd = open_connection("34.246.184.49", 8080, PF_INET, SOCK_STREAM, 0);

        if (strcmp(input, "register\n") == 0)
            register_user();
        else if (strcmp(input, "login\n") == 0)
            login_user();
        else if (strcmp(input, "enter_library\n") == 0)
            enter_library();
        else if (strcmp(input, "get_books\n") == 0)
            get_books();
        else if (strcmp(input, "get_book\n") == 0)
            get_book();
        else if (strcmp(input, "add_book\n") == 0)
            add_book();
        else if (strcmp(input, "delete_book\n") == 0)
            delete_book();
        else if (strcmp(input, "logout\n") == 0)
            logout_user();
        else if (strcmp(input, "exit\n") == 0)
            break;
        else
            printf("ERROR: Command does not exist!\n");
    }

    close_connection(sockfd);

    return 0;
}
